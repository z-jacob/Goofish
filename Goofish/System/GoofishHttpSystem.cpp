#include "GoofishHttpSystem.h"
#include "../Helper/Utils.h"
#include "../Helper/Logger.h"
#include "../Helper/HttpClient.h"
#include "../JSON/CJsonObject.hpp"

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

bool GoofishHttpSystem::Login(std::string cookie, std::string deviceId, std::string& refreshToken, std::string& accessToken)
{

	refreshToken = "";
	accessToken = "";

	LOG_INFO(MODULE_INFO, "cookie:" + cookie);


	auto _m_h5_tk = Utils::ExtractBetween(cookie, "_m_h5_tk=", "_");
	if (_m_h5_tk.length() <= 0)
	{
		LOG_ERROR(MODULE_INFO, "Cookie数据缺失_m_h5_tk参数。");
		return L"";
	}

	auto timeStamp = Utils::GetTimestamp13();

	LOG_INFO(MODULE_INFO, "获取当前设备13位时间戳:" + timeStamp);


	LOG_INFO(MODULE_INFO, "deviceId:" + deviceId);

	auto data = _m_h5_tk + "&" + timeStamp + "&" + m_configModel->appKey + "&" + "{\"appKey\":\"" + m_configModel->appKeyStr + "\",\"deviceId\":\"" + deviceId + "\"}";

	LOG_INFO(MODULE_INFO, "MD5Hash加密前数据:" + data);


	auto sign = Utils::MD5Hash(data);

	LOG_INFO(MODULE_INFO, "MD5Hash:" + sign);


	std::string url = "https://h5api.m.goofish.com/h5/mtop.taobao.idlemessage.pc.login.token/1.0/?jsv=2.7.2&appKey=" + m_configModel->appKey + "&t=" + timeStamp + "&sign=" + sign + "&v=1.0&type=originaljson&accountSite=xianyu&dataType=json&timeout=20000&api=mtop.taobao.idlemessage.pc.login.token&sessionOption=AutoLoginOnly&spm_cnt=a21ybx.im.0.0&spm_pre=a21ybx.home.sidebar.2.4c053da6mkex26&log_id=4c053da6mkex26";

	LOG_INFO(MODULE_INFO, "Login请求URL:" + url);

	std::string body = "data=%7B%22appKey%22%3A%22" + m_configModel->appKeyStr + "%22%2C%22deviceId%22%3A%22" + deviceId + "%22%7D";


	LOG_INFO(MODULE_INFO, "POST提交数据:" + body);


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
			LOG_INFO(MODULE_INFO, "Parse response json fail.");
			return false;
		}


		dataObj.Get("accessToken", accessToken);
		dataObj.Get("refreshToken", refreshToken);

		return !accessToken.empty() && !refreshToken.empty();
	}
	catch (std::exception e)
	{
		std::string exceptionStr(e.what());
		LOG_ERROR(MODULE_INFO, "Login请求异常:" + exceptionStr);
		return false;
	}

}
