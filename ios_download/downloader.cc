#include "ios_cracker/atl_dll_main.h"
#include "googlemock/gmock/internal/gmock-port.h"
#include <atlconv.h>
#pragma comment(lib,"ios_cracker.lib")
#pragma comment(lib,"ios_broker.lib")
#include "smartbot/passport/itunes_client_interface.h"
#include "smartbot/passport/itunes_download_info.h"
#include "ios_download/download_content.h"
#include "ios_broker/pair_downloae_done_plist.h"
#include "ABI\thirdparty\glog\scoped_ptr.h"
#include "utf8_form_js.h"
//google url split lib includes header
#pragma comment(lib,"base.lib")
#pragma comment(lib,"googleurl.lib")
#pragma comment(lib,"icuuc.lib")
#include "googleurl\gurl.h"
#include "gflags\gflags.h"
#include "gflags\util.h"

DEFINE_string(username, "", "account username");
DEFINE_string(password, "", "account password");
DEFINE_string(genfile, "", "generate web passport info file");
DEFINE_string(down_test, "", "yes==download_test||no==gen_passport_file");
DEFINE_string(save_file, "", "test download save path");
DEFINE_string(product_id, "", "test download product id");
DEFINE_string(os_name, "", "authorized os name");
DEFINE_string(os_guid, "", "authorized os guid");

int main(int argc,char** argv,char** env){
	const std::string usage("-username -password -genfile -down_test -save_file -product_id -os_name -os_guid");
	GFLAGS_NAMESPACE::SetVersionString("0.0.1");
	GFLAGS_NAMESPACE::SetUsageMessage(usage);
	GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
	passport::iTunesDownloadInfo* download_info = NULL;
	int result = -1;
	const std::string loc_username = FLAGS_username;
	const std::string loc_password = FLAGS_password;
	const std::string loc_genfile = FLAGS_genfile;
	if(!loc_username.length()||!loc_password.length()){
		GFLAGS_NAMESPACE::ShowUsageWithFlags(argv[0]);
		return -1;
	}
	passport::communicates *communicates = passport::communicates::singleton();
	communicates->ResetSapSetup(true);
	for(int i=0;i<2&&result!=0;i++){
		if(!download_info){
			download_info = passport::iTunesDownloadInfo::GetInterface();
			communicates->DICallerAllocate(download_info);
		}
		if(!FLAGS_os_name.empty()&&!FLAGS_os_guid.empty()){
			if(!communicates->Authenticate(loc_username.c_str(),loc_password.c_str(),FLAGS_os_name.c_str(),FLAGS_os_guid.c_str())){
				Sleep(1000);
				continue;
			}
		}
		else{
			//example:-username kao0626@163.com -password Dd112233 -genfile d:\test.json -down_test yes -save_file d:\\test.ipa -product_id 868013618
			//-username 981902529@qq.com -password ~!QAZ2wsx -genfile C:\Users\appchina\AppData\Local\Temp\auth7345851558765024597.tmp -down_test no
			if(!communicates->Authenticate(loc_username.c_str(),loc_password.c_str())){
				Sleep(1000);
				continue;
			}
			//test:2015/3/18
			//passport::communicates::singleton()->SendMessage_purchase_login();
			//passport::communicates::singleton()->SendMessage_registerSuccess();
			//passport::communicates::singleton()->SendMessage_pendingSongs();
		}
		if(FLAGS_down_test=="yes"){
			//date:2015/09/06
			//communicates->SendMessage_buyProduct(FLAGS_product_id.c_str(), download_info)
			communicates->SendMessageLookupPurchasesAppInfo("375380948");
			if(!communicates->SendMessage_buyProduct(FLAGS_product_id.c_str(),download_info,0,true)){
				Sleep(1000);
				continue;
			}
			USES_CONVERSION;
			const std::wstring cookie = A2W(std::string(std::string("Cookie: downloadKey=")+std::string(download_info->download_key())+std::string("\r\n")).c_str());
			const std::string download_url = std::string(download_info->download_url());
			ios_download::DownloadContent download;
			GURL gurl(download_info->download_url());
			if(!gurl.is_valid()){
				Sleep(1000);
				continue;
			}
			if(!download.CreateConnect(A2W(gurl.host().c_str()),gurl.port().c_str())){
				Sleep(1000);
				continue;
			}
			if(!download.AddDownloadHeader(A2W(gurl.path().c_str()),cookie.c_str())){
				Sleep(1000);
				continue;
			}
			if(download.Download(1,FLAGS_save_file.c_str())!="failed"){
				communicates->SongDownloadDone(FLAGS_product_id.c_str(),download_info);
				result = 0;
				break;
			}
		}
		else if(FLAGS_down_test=="no"){
			if(!FLAGS_os_name.empty()&&!FLAGS_os_guid.empty()){
				if(communicates->GenWebPassportAuthFile(loc_genfile.c_str(),FLAGS_os_name.c_str(),FLAGS_os_guid.c_str())){
					result = 0;
					break;
				}
			}
			else{
				if(communicates->GenWebPassportAuthFile(loc_genfile.c_str())){
					result = 0;
					break;
				}
			}
		}
		else{
			GFLAGS_NAMESPACE::ShowUsageWithFlags(argv[0]);
			break;
		}
	}
	if(download_info){
		communicates->DICallerRelease(download_info);
		download_info = NULL;
	}
	ios_broker::PairDownloaeDonePlist::GetInterface(true);
	passport::iTunesDownloadInfo::GetInterface(true);
	return result;
}
