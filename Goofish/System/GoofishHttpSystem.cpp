#include "GoofishHttpSystem.h"
#include "../Helper/Utils.h"

#include "../Helper/HttpClient.h"
#include "../Helper/CJsonObject.hpp"

void GoofishHttpSystem::OnInit()
{
	m_configModel = this->GetModel<ConfigModel>();
}

void GoofishHttpSystem::OnDeinit()
{
}

void GoofishHttpSystem::OnEvent(std::shared_ptr<JFramework::IEvent> event)
{
}

bool GoofishHttpSystem::refresh_token(std::string cookie, std::string deviceId, std::string& refreshToken, std::string& accessToken)
{

	refreshToken = "";
	accessToken = "";

	Log(MODULE_INFO, "开始刷新token... (滑块验证重试次数: 0)");

	Log(MODULE_INFO, "开始执行Cookie刷新任务...");

	Log(MODULE_INFO, "========== Token刷新API调用详情 ==========");

	Log(MODULE_INFO, "API接口:https://h5api.m.goofish.com/h5/mtop.taobao.idlemessage.pc.login.token/1.0/⁠");


	Log(MODULE_INFO, "========== 签名计算信息 ==========");


	auto _m_h5_tk = Utils::ExtractBetween(cookie, "_m_h5_tk=", "_");
	if (_m_h5_tk.length() <= 0)
	{
		LogError(MODULE_INFO, "Cookie数据缺失_m_h5_tk参数。");
		return L"";
	}


	Log(MODULE_INFO, "token (从_m_h5_tk提取): " + _m_h5_tk + " (长度: " + std::to_string(_m_h5_tk.length()) + ")");


	auto timeStamp = Utils::GetTimestamp13();

	Log(MODULE_INFO, "timestamp (t): " + timeStamp);

	Log(MODULE_INFO, "appKey: " + m_configModel->appKey);

	auto data_val = "{\"appKey\":\"" + m_configModel->appKeyStr + "\",\"deviceId\":\"" + deviceId + "\"}";

	Log(MODULE_INFO, "data_val: " + data_val);

	auto data = _m_h5_tk + "&" + timeStamp + "&" + m_configModel->appKey + "&" + data_val;

	Log(MODULE_INFO, "计算签名: MD5(" + data + ")");


	auto sign = Utils::MD5Hash(data);

	Log(MODULE_INFO, "最终签名: " + sign);


	std::string url = "https://h5api.m.goofish.com/h5/mtop.taobao.idlemessage.pc.login.token/1.0/?jsv=2.7.2&appKey=" + m_configModel->appKey + "&t=" + timeStamp + "&sign=" + sign + "&v=1.0&type=originaljson&accountSite=xianyu&dataType=json&timeout=20000&api=mtop.taobao.idlemessage.pc.login.token&sessionOption=AutoLoginOnly&spm_cnt=a21ybx.im.0.0&spm_pre=a21ybx.home.sidebar.2.4c053da6mkex26&log_id=4c053da6mkex26";

	Log(MODULE_INFO, "登录URL:" + url);


	std::string body = "data=%7B%22appKey%22%3A%22" + m_configModel->appKeyStr + "%22%2C%22deviceId%22%3A%22" + deviceId + "%22%7D";


	Log(MODULE_INFO, "登录提交数据:" + body);


	std::vector<std::pair<std::wstring, std::wstring>> headers;
	headers.emplace_back(L"User-Agent", L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36");
	headers.emplace_back(L"Accept", L"application/json");
	headers.emplace_back(L"Content-Type", L"application/x-www-form-urlencoded");
	headers.emplace_back(L"Origin", L"https://www.goofish.com");
	headers.emplace_back(L"Referer", L"https://www.goofish.com/");
	headers.emplace_back(L"Cookie", Utils::StringToWString(cookie));

	try
	{
		HttpClient::Response resp;
		bool ok = HttpClient::Post(Utils::StringToWString(url), body, resp, headers, 20000);
		neb::CJsonObject root(resp.body);

		neb::CJsonObject dataObj;
		if (!root.Get("data", dataObj))
		{
			LogError(MODULE_INFO, "登录数据解析失败");
			return false;
		}


		dataObj.Get("accessToken", accessToken);
		dataObj.Get("refreshToken", refreshToken);

		return !accessToken.empty() && !refreshToken.empty();
	}
	catch (std::exception e)
	{
		LogError(MODULE_INFO, "登录异常:" + Utils::ToString(e.what()));
	}
	return false;
}
