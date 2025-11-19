#include "FontModel.h"

void FontModel::OnInit()
{
	m_font.CreateFont(
		13,                // 字体高度
		0,                 // 字体宽度（0为自适应）
		0,                 // 文字倾斜角度
		0,                 // 基线倾斜角度
		FW_NORMAL,           // 字体粗细
		FALSE,             // 是否斜体
		FALSE,             // 是否下划线
		0,                 // 字体字符集
		ANSI_CHARSET,      // 字符集
		OUT_DEFAULT_PRECIS,// 输出精度
		CLIP_DEFAULT_PRECIS,// 裁剪精度
		DEFAULT_QUALITY,   // 输出质量
		DEFAULT_PITCH | FF_SWISS, // 字体类型
		_T("宋体")      // 字体名称
	);
}

void FontModel::OnDeinit()
{
}
