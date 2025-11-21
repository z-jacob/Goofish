#include "GoofishHttp.h"
#include "HttpClient.h"

#include "../Helper/Logger.h"

std::string GoofishHttp::Login(std::wstring cookie)
{
	LOG_INFO(MODULE_INFO, "Cookie:" + Utils::WStringToString(cookie));


	//t=5e247d2ff89af2e56b4aeebfdb3ef501; cna=9/CGIWtgdRkCAQAAAAAO+iAb; tracknick=yllove1989; havana_lgc2_77=eyJoaWQiOjI3MDM5MjM0NTAsInNnIjoiZDY0OTFiNzUzZTliNjNjZmZiMjI0MzE1NmQ3ZmZkZDciLCJzaXRlIjo3NywidG9rZW4iOiIxNEZpQlFBcVJhNGpOU2tSZ0hjY0V6QSJ9; _hvn_lgc_=77; havana_lgc_exp=1764606397378; cookie2=10d4943562d7d9d07a2f1aa15ef2ee45; mtop_partitioned_detect=1; _m_h5_tk=dbb28585a54083987ab5c77e052d1473_1763739200056; _m_h5_tk_enc=40080f90d65d4d8fd43c145d5a7bbfba; xlly_s=1; _samesite_flag_=true; sgcookie=E100Vfm6R5sj9ZSJJGsuFO4JzXjtZoK4nA60hqz3Dci0klrN7pm%2BK9ck12doRC2Q7of7r8ty%2FX3aROzVhJR4Gj5UokOqz5bc5isdiWT68Wmb8K0%3D; csg=dcfb326c; _tb_token_=315a1e433b86b; unb=2703923450; sdkSilent=1763816242049; tfstk=grDraaxSjLpP1SQXFqeFbfDmx4e8EJ81YvaQxDm3Vz4lFT_ngV0YPXG7PMlq-qF7rbi52j3sjTMWwJ6UeJeH5F96C0i8pJDA2l8YymmYxnZk5YYOVJeH5E1fK7pap48UNW2nmnrQA_qn-_XD0uEUKTVhEsb0kk23KzV3niqUXyjux7xqmrE3Ky03Kny0kk23-22nwRFLNsauZn1XFZ_FMpNgS04VKOST47DHBrWHExrr0Pmoj9Xn3uPrnd0oUxVStcFtFDJFH8iZi-qs2LW4I0qZPrgMItymcm0Q6jLdCS3qL5HuwMfo0vzzsYVVAOn43k03TjLNCuySg5krwHQmqVars8nCbUi4_jPTmSjFZ8GsfvFiEL7Tl5EEPrgMIty0tg85JovHOvhP-6PuDoz60n5WiDxRjYMu-6CLiZq45hz196Fkyoz6cT1d9Sf00PtrY



	//dbb28585a54083987ab5c77e052d1473&1763733511356&34839810&{"appKey":"444e9908a51d1cb236a27862abc769c9","deviceId":"D5C2B837-1EA7-427B-8110-2DC5F27B911D-2703923450"}


	//d.token + "&"+j+"&"+h+"&"+c.data


	std::wstring url = L"https://h5api.m.goofish.com/h5/mtop.taobao.idlemessage.pc.login.token/1.0/?jsv=2.7.2&appKey=34839810&t=1763729847770&sign=f03ba42e5d42c941431cc59da15bc638&v=1.0&type=originaljson&accountSite=xianyu&dataType=json&timeout=20000&api=mtop.taobao.idlemessage.pc.login.token&sessionOption=AutoLoginOnly&spm_cnt=a21ybx.im.0.0&spm_pre=a21ybx.home.sidebar.2.4c053da6mkex26&log_id=4c053da6mkex26";

	// body is already url-encoded in the provided example
	std::string body = "data=%7B%22appKey%22%3A%22444e9908a51d1cb236a27862abc769c9%22%2C%22deviceId%22%3A%22610D72D4-FEC8-4D44-8F1C-1838A2DBF8A4-2703923450%22%7D";

	std::vector<std::pair<std::wstring, std::wstring>> headers;
	headers.emplace_back(L"User-Agent", L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36");
	headers.emplace_back(L"Accept", L"application/json");
	headers.emplace_back(L"Content-Type", L"application/x-www-form-urlencoded");
	headers.emplace_back(L"Origin", L"https://www.goofish.com");
	headers.emplace_back(L"Referer", L"https://www.goofish.com/");


	headers.emplace_back(L"Cookie", cookie);

	HttpClient::Response resp;
	bool ok = HttpClient::Post(url, body, resp, headers, 20000);

	if (ok) {
		LOG_INFO(MODULE_INFO, "pc.login.token sucess#\nstatus: " + std::to_string(resp.status) + "\nBody size:" + std::to_string(resp.body.size()));
		return resp.body;
	}
	else {
		LOG_ERROR(MODULE_INFO, "pc.login.token failed");
		return "";
	}
}
