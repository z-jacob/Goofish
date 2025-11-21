#include "GoofishHttp.h"
#include "HttpClient.h"

#include "../Helper/Logger.h"

std::wstring GoofishHttp::Login(std::wstring cookie)
{
	LOG_INFO(MODULE_INFO, "Cookie:" + Utils::WStringToString(cookie));


	auto _m_h5_tk = Utils::ExtractBetween(Utils::WStringToString(cookie), "_m_h5_tk=", "_");
	if (_m_h5_tk.length() <= 0)
	{
		LOG_ERROR(MODULE_INFO, "Cookie数据缺失_m_h5_tk参数。");
		return L"";
	}

	auto timeStamp = Utils::GetTimestamp13();

	LOG_INFO(MODULE_INFO, "获取当前设备13位时间戳:" + timeStamp);

	std::string appKey = "34839810";

	LOG_INFO(MODULE_INFO, "appKey:" + appKey);

	std::string appKeyStr = "444e9908a51d1cb236a27862abc769c9";

	LOG_INFO(MODULE_INFO, "appKeyStr:" + appKeyStr);

	std::string deviceId = "D5C2B837-1EA7-427B-8110-2DC5F27B911D-2703923450";

	LOG_INFO(MODULE_INFO, "deviceId:" + deviceId);

	auto data = _m_h5_tk + "&" + timeStamp + "&" + appKey + "&" + "{\"appKey\":\"" + appKeyStr + "\",\"deviceId\":\"" + deviceId + "\"}";

	LOG_INFO(MODULE_INFO, "MD5Hash加密前数据:" + data);


	auto sign = Utils::MD5Hash(data);

	LOG_INFO(MODULE_INFO, "MD5Hash:" + sign);


	std::string url = "https://h5api.m.goofish.com/h5/mtop.taobao.idlemessage.pc.login.token/1.0/?jsv=2.7.2&appKey=" + appKey + "&t=" + timeStamp + "&sign=" + sign + "&v=1.0&type=originaljson&accountSite=xianyu&dataType=json&timeout=20000&api=mtop.taobao.idlemessage.pc.login.token&sessionOption=AutoLoginOnly&spm_cnt=a21ybx.im.0.0&spm_pre=a21ybx.home.sidebar.2.4c053da6mkex26&log_id=4c053da6mkex26";

	LOG_INFO(MODULE_INFO, "Login请求URL:" + url);

	std::string body = "data=%7B%22appKey%22%3A%22" + appKeyStr + "%22%2C%22deviceId%22%3A%22" + deviceId + "%22%7D";


	LOG_INFO(MODULE_INFO, "POST提交数据:" + body);


	std::vector<std::pair<std::wstring, std::wstring>> headers;
	headers.emplace_back(L"User-Agent", L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36");
	headers.emplace_back(L"Accept", L"application/json");
	headers.emplace_back(L"Content-Type", L"application/x-www-form-urlencoded");
	headers.emplace_back(L"Origin", L"https://www.goofish.com");
	headers.emplace_back(L"Referer", L"https://www.goofish.com/");


	headers.emplace_back(L"Cookie", cookie);

	HttpClient::Response resp;
	bool ok = HttpClient::Post(Utils::StringToWString(url), body, resp, headers, 20000);
	LOG_INFO(MODULE_INFO, "Response: " + Utils::WStringToString(Utils::StringToWString(resp.body)));
	return Utils::StringToWString(resp.body);
}
