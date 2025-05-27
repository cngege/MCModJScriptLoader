//import * as 工具 from '工具'
import * as 事件系统 from '事件系统'
import SignCode from 'SignCode'
import * as http from 'http'
import * as ImGui from 'imgui'

(function(模块 : 类型_公共模块){
    const logger = new spdlog();

    let 测试值 = 0;
    let 下拉框测试值 = 0;

    let Color : color_3 = {
        r: 1.0,
        g: 0.0,
        b: 0.0
    }
    let 单选按钮 = 0;
    let 下拉列表选择 = -1;
    let 选择框多选 = [false, false, false, false];
    let 输入的文本 = "A";

    事件系统.监听事件("onRender", () => {
        if(!模块.enable) return;
        if(ImGui.Begin("测试窗口", (_ = 模块.enable) => 模块.enable = _)){
            // Old Data Time
            if(ImGui.CollapsingHeader("old time")){
                ImGui.SliderInt("测试输入", (_ = 测试值)=> 测试值 = _, -10, 10);
                ImGui.Text("测试值: " , 测试值);
                ImGui.SliderFloat("测试浮点输入", (_ = 测试值)=> 测试值 = _, -10.0, 10.0);
                ImGui.Combo("测试下拉框", (_ = 下拉框测试值) => 下拉框测试值 = _, ["选项1", "选项2", "选项3"]);
                ImGui.ColorEdit3("测试颜色", Color);
                ImGui.RadioButton("单选按钮1", (_ = 单选按钮) => 单选按钮 = _, 0);
                ImGui.RadioButton("单选按钮2", (_ = 单选按钮) => 单选按钮 = _, 1);
                ImGui.ProgressBar(测试值 / 10, { x: -1, y: 0 }, "进度条测试");
            }

            // 5 - 28
            if(ImGui.CollapsingHeader("5/28")){
                ImGui.ListBox("下拉列表框",(_ = 下拉列表选择)=>下拉列表选择 = _, ["AAAA","CCCC","DDDD","BBBB"])
                ImGui.InputText("输入文本",32, (_ = 输入的文本)=> 输入的文本 = _);
                if(ImGui.TreeNode("root")){
                    if(ImGui.Selectable("选择框A1",下拉列表选择 == 0)) 下拉列表选择 = 0;
                    if(ImGui.Selectable("选择框A2",下拉列表选择 == 1)) 下拉列表选择 = 1;
                    if(ImGui.Selectable("选择框A3",下拉列表选择 == 2)) 下拉列表选择 = 2;
                    if(ImGui.Selectable("选择框A4",下拉列表选择 == 3)) 下拉列表选择 = 3;
                    ImGui.Selectable("选择框B1", (_ = 选择框多选[0]) => 选择框多选[0] = _);
                    ImGui.Selectable("选择框B2", (_ = 选择框多选[1]) => 选择框多选[1] = _);
                    ImGui.Selectable("选择框B3", (_ = 选择框多选[2]) => 选择框多选[2] = _);
                    ImGui.Selectable("选择框B4", (_ = 选择框多选[3]) => 选择框多选[3] = _);
                    ImGui.TreePop();
                }
            }
        }
        ImGui.End();
    })

    logger.info("已成功加载模块:", 模块.name);
})(__模块__)