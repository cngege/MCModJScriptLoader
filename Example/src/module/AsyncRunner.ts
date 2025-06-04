/**
 * AsyncRunner 模块 - 完整实现
 * 提供基于生成器的异步流程控制，支持取消和超时
 */

// ====================== 类型定义 ======================
/**
 * 异步生成器函数的类型定义
 * @typeParam TInput - 生成器输入类型（next() 接受的值）
 * @typeParam TReturn - 生成器返回值类型
 */
export type AsyncGeneratorFn<TInput, TReturn> = 
  (...args: any[]) => Generator<TInput | Promise<TInput> | AbortSignal, TReturn, TInput>;

/**
 * 可取消的异步任务结果
 * @typeParam T - 结果数据的类型
 */
export type CancelableResult<T = any> = {
  /**
   * 表示任务状态的Promise
   * - 成功时解析为任务结果
   * - 失败或取消时被拒绝
   */
  promise: Promise<T>;
  
  /**
   * 取消任务的方法
   * @param reason - 可选的原因描述
   */
  cancel: (reason?: string) => void;
};

/**
 * AsyncRunner 的配置选项
 */
export interface AsyncRunnerOptions {
  /**
   * 任务超时时间（毫秒）
   */
  timeout?: number;
  
  /**
   * 是否在取消时抛出错误（默认为true）
   */
  throwOnCancel?: boolean;
}

/**
 * 增强的取消错误类
 */
export class CancellationError extends Error {
  constructor(message: string = 'Operation cancelled') {
    super(message);
    this.name = 'CancellationError';
  }
}

// ====================== AsyncRunner 类实现 ======================
/**
 * 异步生成器执行器
 * 允许执行生成器函数并支持取消和超时
 */
export class AsyncRunner<TInput = any, TReturn = any> {
  private generator: Generator<TInput | Promise<TInput> | AbortSignal, TReturn, TInput>;
  private isRunning = false;
  private isCancelled = false;
  private cancelReason?: string;
  private resolve!: (value: TReturn | PromiseLike<TReturn>) => void;
  private reject!: (reason?: any) => void;
  private controller: AbortController;
  private timeoutId?: number;
  
  /**
   * 创建一个新的AsyncRunner实例
   * @param generatorFn - 要执行的异步生成器函数
   */
  constructor(private generatorFn: AsyncGeneratorFn<TInput, TReturn>) {
    this.generator = generatorFn();
    this.controller = new AbortController();
  }

  /**
   * 启动异步流程
   * @param options - 配置选项
   * @returns 可取消的结果对象
   */
  run(options: AsyncRunnerOptions = {}): CancelableResult<TReturn> {
    if (this.isRunning) {
      throw new Error('AsyncRunner is already running');
    }

    const { timeout, throwOnCancel = true } = options;
    
    this.isRunning = true;
    this.isCancelled = false;
    this.cancelReason = undefined;
    
    // 设置超时（如果配置了）
    if (timeout && timeout > 0) {
      this.timeoutId = setTimeout(() => {
        this.cancel(`Operation timed out after ${timeout}ms`);
      }, timeout) as unknown as number;
    }
    
    return {
      promise: new Promise<TReturn>((resolve, reject) => {
        this.resolve = resolve;
        this.reject = reject;
        this.processStep(() => this.generator.next());
      }),
      cancel: (reason = 'Operation cancelled by user') => this.cancel(reason)
    };
  }

  /**
   * 获取取消信号
   */
  get signal(): AbortSignal {
    return this.controller.signal;
  }

  /**
   * 取消运行
   * @param reason - 取消原因
   */
  private cancel(reason: string): void {
    if (!this.isRunning || this.isCancelled) return;
    
    this.isCancelled = true;
    this.cancelReason = reason;
    this.controller.abort(reason);
    
    // 清除超时计时器
    if (this.timeoutId) {
      clearTimeout(this.timeoutId);
      this.timeoutId = undefined;
    }
    
    this.tryCancel();
  }

  /**
   * 处理步骤逻辑
   * @param nextFn - 获取下一个迭代结果的函数
   */
  private processStep(nextFn: () => IteratorResult<TInput | Promise<TInput> | AbortSignal, TReturn>): void {
    if (this.isCancelled) {
      this.tryCancel();
      return;
    }

    try {
      const result = nextFn();
      
      if (result.done) {
        this.complete(result.value);
        return;
      }

      const value = result.value;
      
      if (this.isPromise(value)) {
        this.handlePromise(value as Promise<TInput>);
      } else if (this.isAbortSignal(value)) {
        this.handleAbortSignal(value);
      } else {
        // 处理直接值
        this.handleValue(value as TInput);
      }
    } catch (error) {
      this.handleError(error);
    }
  }

  /**
   * 处理直接值
   * @param value - 生成器产生的直接值
   */
  private handleValue(value: TInput): void {
    this.processStep(() => this.generator.next(value));
  }

  /**
   * 处理Promise值
   * @param promise - 生成器产生的Promise
   */
  private handlePromise(promise: Promise<TInput>): void {
    promise.then(
      resolvedValue => {
        if (this.isCancelled) {
          this.tryCancel();
          return;
        }
        this.processStep(() => this.generator.next(resolvedValue));
      },
      error => {
        if (this.isCancelled) {
          this.tryCancel();
          return;
        }
        this.processStep(() => this.generator.throw(error));
      }
    );
  }

  /**
   * 处理AbortSignal
   * @param signal - 生成器产生的AbortSignal
   */
  private handleAbortSignal(signal: AbortSignal): void {
    // 如果信号已经中止，则立即取消
    if (signal.aborted) {
      this.tryCancel();
      return;
    }
    
    // 监听中止事件
    const onAbort = () => {
      signal.removeEventListener('abort', onAbort);
      this.cancel('Abort signal received');
    };
    
    signal.addEventListener('abort', onAbort);
    
    // 继续流程
    this.processStep(() => this.generator.next());
  }

  /**
   * 处理错误
   * @param error - 抛出的错误
   */
  private handleError(error: unknown): void {
    try {
      this.processStep(() => this.generator.throw(error));
    } catch (innerError) {
      this.fail(innerError);
    }
  }

  /**
   * 完成任务
   * @param value - 生成器返回的值
   */
  private complete(value: TReturn): void {
    this.cleanup();
    this.resolve(value);
  }

  /**
   * 任务失败处理
   * @param reason - 失败原因
   */
  private fail(reason: unknown): void {
    this.cleanup();
    this.reject(reason);
  }

  /**
   * 尝试取消任务
   */
  private tryCancel(): void {
    this.cleanup();
    
    if (this.isCancelled && this.cancelReason) {
      const error = new CancellationError(this.cancelReason);
      this.reject(error);
    }
  }
  
  /**
   * 清理资源
   */
  private cleanup(): void {
    this.isRunning = false;
    
    if (this.timeoutId) {
      clearTimeout(this.timeoutId);
      this.timeoutId = undefined;
    }
  }

  /**
   * 类型守卫 - Promise
   */
  private isPromise(value: unknown): value is Promise<unknown> {
    return !!value && typeof (value as Promise<unknown>).then === 'function';
  }

  /**
   * 类型守卫 - AbortSignal
   */
  private isAbortSignal(value: unknown): value is AbortSignal {
    const signal = value as AbortSignal;
    return !!signal && 
           typeof signal.aborted === 'boolean' && 
           typeof signal.addEventListener === 'function';
  }
}

// ====================== 辅助函数 ======================
/**
 * 创建一个可取消的异步任务
 * @param generatorFn - 异步生成器函数
 * @param options - 运行选项
 * @returns 可取消的结果对象
 */
export function createAsyncTask<TInput, TReturn>(
  generatorFn: AsyncGeneratorFn<TInput, TReturn>, 
  options?: AsyncRunnerOptions
): CancelableResult<TReturn> {
  const runner = new AsyncRunner(generatorFn);
  return runner.run(options);
}
