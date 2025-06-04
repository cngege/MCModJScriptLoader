import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
import SignCode from 'SignCode'
import * as http from 'http'
import * as ImGui from 'imgui'
import ClientInstance from './module/SDK/ClientInstance'
import MinecraftGame from './module/SDK/MinecraftGame'
import GuiData from './module/SDK/GuiData'
//import ClientInstance from 'https://disk.ningsuan.com.cn/index.php?explorer/share/file&hash=5365IsSFHRgVR5UvNiDNr8HqFniXKLi32dWpekJJ6CTagNaJHBpLObWgeRcwdINF4DJzLQLGcQ&name=/ClientInstance.js&_etag=bc55b&size=1549'

(function(模块 : 类型_公共模块){
    const logger = new spdlog();

    
    事件系统.监听事件("onSave",function(data : any){
        data[模块.name] = {
            enable: 模块.enable
        };
        return true;
    });

    事件系统.监听事件("onLoad",function(data : any){
        let thisData = data[模块.name];
        if(thisData){
            模块.enable = thisData.enable;
        }
    });



    let Color : color_3 = {
        r: 1.0,
        g: 0.0,
        b: 0.0
    }
    let UIVar = {
        测试值: 0,
        下拉框测试值: 0,

        单选按钮 : 0,
        下拉列表选择: -1,
        选择框多选 : [false, false, false, false],
        输入的文本 : "A",
        显示鼠标位置: false,
        显示Fov: false,
        显示屏幕大小: false,
        显示MG地址相关信息: false,
        显示GuiData相关信息: false,
    }


    事件系统.监听事件("onRender", () => {
        if(!模块.enable) return;
        if(ImGui.Begin("测试窗口", (_ = 模块.enable) => 模块.enable = _)){
            // Old Data Time
            if(ImGui.CollapsingHeader("old time")){
                ImGui.SliderInt("测试输入", (_ = UIVar.测试值)=> UIVar.测试值 = _, -10, 10);
                ImGui.Text("测试值: " , UIVar.测试值);
                ImGui.SliderFloat("测试浮点输入", (_ = UIVar.测试值)=> UIVar.测试值 = _, -10.0, 10.0);
                ImGui.Combo("测试下拉框", (_ = UIVar.下拉框测试值) => UIVar.下拉框测试值 = _, ["选项1", "选项2", "选项3"]);
                ImGui.ColorEdit3("测试颜色", Color);
                ImGui.RadioButton("单选按钮1", (_ = UIVar.单选按钮) => UIVar.单选按钮 = _, 0);
                ImGui.RadioButton("单选按钮2", (_ = UIVar.单选按钮) => UIVar.单选按钮 = _, 1);
                ImGui.ProgressBar(UIVar.测试值 / 10, { x: -1, y: 0 }, "进度条测试");
            }

            // 5 - 28
            if(ImGui.CollapsingHeader("5/28")){
                ImGui.ListBox("下拉列表框",(_ = UIVar.下拉列表选择)=>UIVar.下拉列表选择 = _, ["AAAA","CCCC","DDDD","BBBB"])
                ImGui.InputText("输入文本",32, (_ = UIVar.输入的文本)=> UIVar.输入的文本 = _);
                if(ImGui.TreeNode("root")){
                    if(ImGui.Selectable("选择框A1",UIVar.下拉列表选择 == 0)) UIVar.下拉列表选择 = 0;
                    if(ImGui.Selectable("选择框A2",UIVar.下拉列表选择 == 1)) UIVar.下拉列表选择 = 1;
                    if(ImGui.Selectable("选择框A3",UIVar.下拉列表选择 == 2)) UIVar.下拉列表选择 = 2;
                    if(ImGui.Selectable("选择框A4",UIVar.下拉列表选择 == 3)) UIVar.下拉列表选择 = 3;
                    ImGui.Selectable("选择框B1", (_ = UIVar.选择框多选[0]) => UIVar.选择框多选[0] = _);
                    ImGui.Selectable("选择框B2", (_ = UIVar.选择框多选[1]) => UIVar.选择框多选[1] = _);
                    ImGui.Selectable("选择框B3", (_ = UIVar.选择框多选[2]) => UIVar.选择框多选[2] = _);
                    ImGui.Selectable("选择框B4", (_ = UIVar.选择框多选[3]) => UIVar.选择框多选[3] = _);
                    ImGui.TreePop();
                }
            }
            if(ImGui.CollapsingHeader("5/31")){
                if(ImGui.Button("复制##ClientInstance")){
                    ImGui.SetClipboardText("0x"+ClientInstance.Ptr.toString(16));
                }
                ImGui.SameLine(); ImGui.Text("CI: " , ClientInstance.Ptr.toString(16));
                ImGui.Text("VTF: " , new NativePoint(ClientInstance.Ptr).getulong().toString(16));
                ImGui.SameLine();
                ImGui.SameLine(); ImGui.Text("VTF2: " , ClientInstance.Vtf.toString(16));

                ImGui.SeparatorText("鼠标位置");
                ImGui.Checkbox("显示鼠标位置", (_ = UIVar.显示鼠标位置) => UIVar.显示鼠标位置 = _);
                if(UIVar.显示鼠标位置){
                    ImGui.Text("鼠标位置: " + JSON.stringify(ClientInstance.GetMousePos()));
                }

                ImGui.SeparatorText("Fov");
                ImGui.Checkbox("显示Fov", (_ = UIVar.显示Fov) => UIVar.显示Fov = _);
                if(UIVar.显示Fov){
                    ImGui.Text("Fov: " + JSON.stringify(ClientInstance.GetFov()));
                }
                
                ImGui.SeparatorText("屏幕大小");
                ImGui.Checkbox("显示屏幕大小", (_ = UIVar.显示屏幕大小) => UIVar.显示屏幕大小 = _);
                if(UIVar.显示屏幕大小){
                    ImGui.Text("SizeSize: " + JSON.stringify(ClientInstance.GetScreenSize()));
                }

                ImGui.SeparatorText("MinecraftGame");
                ImGui.Checkbox("显示MG相关信息", (_ = UIVar.显示MG地址相关信息) => UIVar.显示MG地址相关信息 = _);
                if(UIVar.显示MG地址相关信息){
                    ImGui.SeparatorText("来自CI");
                    if(ImGui.Button("复制##MinecraftGame")){
                        ImGui.SetClipboardText("0x"+ClientInstance.GetMinecraftGame().toNumber().toString(16));
                    }
                    ImGui.SameLine(); ImGui.Text("MG: " , ClientInstance.GetMinecraftGame().toNumber().toString(16));
                    ImGui.Text("VTF: " , ClientInstance.GetMinecraftGame().getulong().toString(16));
                    ImGui.SeparatorText("来自基址");
                    if(ImGui.Button("复制##MinecraftGame_base")){
                        ImGui.SetClipboardText("0x"+MinecraftGame.Ptr.toNumber().toString(16));
                    }
                    ImGui.SameLine(); ImGui.Text("MG: " , MinecraftGame.Ptr.toNumber().toString(16));
                }

                ImGui.SeparatorText("GuiData");
                ImGui.Checkbox("显示GuiData相关信息", (_ = UIVar.显示GuiData相关信息) => UIVar.显示GuiData相关信息 = _);
                if(UIVar.显示GuiData相关信息){
                    if(ImGui.Button("复制##GuiData")){
                        ImGui.SetClipboardText("0x"+ClientInstance.GetGuiData().toNumber().toString(16));
                    }
                    ImGui.SameLine(); ImGui.Text("GuiData: " , ClientInstance.GetGuiData().toNumber().toString(16));
                    ImGui.Text("VTF: " , ClientInstance.GetGuiData().getulong().toString(16));
                }
                if(ImGui.Button("打印ClientInstance::getTopScreenName")){
                    logger.info(ClientInstance.GetTopScreenName());
                }
            }

            if(ImGui.CollapsingHeader("6/2")){
                if(ImGui.Button("输出客户端文本消息1")){
                    ClientInstance.GetGuiData().displayClientMessage("测试输出文本消息测试输出文本消息测试输出文本消息测试输出文本消息测试输出文本消息测试输出文本消息测试输出文本消息");
                }
                if(ImGui.Button("输出客户端文本消息2")){
                    ClientInstance.GetGuiData().displayClientMessage("测试输出文本消息 %s", "第二个消息");
                }
                if(ImGui.Button("text")){
                    let x = new NativePoint(0,32);
                    x.onfree((v)=>{
                        logger.info("X 被释放");
                    });
                    
                }
            }
        }
        ImGui.End();
    });

    logger.info("已成功加载模块:", 模块.name);
})(__模块__)

/**
ArrayBuffer 构造函数            ✅          new ArrayBuffer(10)
TypedArray 视图                 ✅          new Uint8Array(buffer)
DataView                        ✅          new DataView(buffer)
字节操作和转换                  ✅           new Float32Array(buffer)
子数组切片                      ✅          buffer.slice(2, 8)
ArrayBuffer.isView()            ✅          ArrayBuffer.isView(new Uint8Array())
.byteLength 属性                ✅          buffer.byteLength
.maxByteLength 属性 (ES2020)    ✅          new ArrayBuffer(10, {maxByteLength: 20})
.grow()/.resize() 方法	        ✅          buffer.grow(15)
 */

// 48 8D 05 ? ? ? ? 48 89 ? BA ? ? ? ? 44
// 使用上述地址找，能找到两个，一般第一个是本地玩家析构地址对应的虚表位置，第二个是服务玩家析构地址对应的虚表位置

// 本地玩家虚表地址
/**
 * 48 8D 05 ? ? ? ? 49 89 04 ? 49 8D 8C +3 (应该是构造函数)
 * 48 8D 05 ? ? ? ? 48 89 ? BA ? ? ? ? 44 8D 4A ? 44 8D 42 ? 66 C7 44 +3（应该是析构函数）
 */

// 服务玩家虚表地址
/**
 * 48 8D 05 ? ? ? ? 48 89 ? 8B 85 ? ? ? ? 89 87 +3
 * 
 */

//void* CI::TICK(ci*)
// 40 53 48 83 EC ? 48 8B ? 48 8B 89 ? ? ? ? 48 8B ? 48 8B 80 ? ? ? ? FF ? ? ? ? ? 84 ? 75 ? 48