#include <iostream>
#include <map>
#include "SpeedTest.h"
#include "TestConfigTemplate.h"


void banner(){
    std::cout << "SpeedTest++ version " << SpeedTest_VERSION_MAJOR << "." << SpeedTest_VERSION_MINOR << std::endl;
    std::cout << "Speedtest.net command line interface" << std::endl;
    std::cout << "Info: " << SpeedTest_HOME_PAGE << std::endl;
    std::cout << "Author: " << SpeedTest_AUTHOR << std::endl;
}

void usage(const char* name){
    std::cout << "usage: " << name << " ";
    std::cout << "[--latency] [--download] [--upload] [--help] [--share]" << std::endl;
    std::cout << "optional arguments:" << std::endl;
    std::cout << "\t--help      Show this message and exit\n";
    std::cout << "\t--latency   Perform latency test only\n";
    std::cout << "\t--download  Perform download test only. It includes latency test\n";
    std::cout << "\t--upload    Perform upload test only. It includes latency test\n";
    std::cout << "\t--share     Generate and provide a URL to the speedtest.net share results image\n";
    std::cout << "\t--verbose   Show verbose output\n";
}

int main(const int argc, const char **argv) {

    bool download_only = false;
    bool upload_only   = false;
    bool latency_only  = false;
    bool share         = false;
	bool verbose		= false; 
    std::vector<std::string> options;

    for (int i = 0; i < argc; i++)
        options.push_back(std::string(argv[i]));

    if(verbose) {
		banner();
    	std::cout << std::endl;
	}

    if (std::find(options.begin(), options.end(), "--help") != options.end()) {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (std::find(options.begin(), options.end(), "--latency") != options.end())
        latency_only = true;

    if (std::find(options.begin(), options.end(), "--download") != options.end())
        download_only = true;

    if (std::find(options.begin(), options.end(), "--upload") != options.end())
        upload_only = true;

    if (std::find(options.begin(), options.end(), "--share") != options.end())
        share = true;

    if (std::find(options.begin(), options.end(), "--verbose") != options.end())
        verbose = true;


    auto sp = SpeedTest();
    IPInfo info;
    if (!sp.ipInfo(info)){
        std::cerr << "Unable to retrieve your IP info. Try again later" << std::endl;
        return EXIT_FAILURE;
    }

	if(verbose) {
		std::cout << "IP: " << info.ip_address
			<< " ( " << info.isp << " ) "
			<< "Location: [" << info.lat << ", " << info.lon << "]" << std::endl;

		std::cout << "Finding fastest server... " << std::flush;
	}

    auto serverList = sp.serverList();
    if (serverList.empty()){
        std::cerr << "Unable to download server list. Try again later" << std::endl;
        return EXIT_FAILURE;
    }

	if(verbose)
    	std::cout << serverList.size() << " Servers online" << std::endl;


    ServerInfo serverInfo = sp.bestServer(10, [](bool success) {
        //if(verbose) std::cout << (success ? '.' : '*') << std::flush;
    });

    if(verbose) std::cout << std::endl;

    std::cout << "server " << serverInfo.name << std::endl
        << "sponsor " << serverInfo.sponsor << std::endl
        << "server_distance_km " << serverInfo.distance << std::endl
        << "server_latency_ms " << sp.latency() << std::endl;

    long jitter = 0;
    if (sp.jitter(serverInfo, jitter)){
    	std::cout << "jitter_ms " << jitter << std::endl;
    } 

    if(verbose) std::cout << "Finding fastest server for packet loss measurement... " << std::flush;
    auto serverQualityList = sp.serverQualityList();

    if (serverQualityList.empty()){
        std::cerr << "Unable to download server list. Packet loss analysis is not available at this time" << std::endl;
    } else {
        if(verbose) std::cout << serverQualityList.size() << " Ping hosts online" << std::endl;
        ServerInfo serverQualityInfo = sp.bestQualityServer(5, [](bool success){
            //if(verbose) std::cout << (success ? '.' : '*') << std::flush;
        });
		if(verbose) {	
			std::cout << std::endl;
			std::cout << "Server: " << serverQualityInfo.name
			<< " by " << serverQualityInfo.sponsor
			<< " (" << serverQualityInfo.distance << " km from you): " << std::endl;
		}
        int ploss = 0;
        if (sp.packetLoss(serverQualityInfo, ploss)){
            std::cout << "packet_loss " << ploss << std::endl;
        }
    }

    if (latency_only)
        return EXIT_SUCCESS;


    if(verbose) std::cout << "Determine line type (" << preflightConfigDownload.concurrency << ") "  << std::flush;
    double preSpeed = 0;
    if (!sp.downloadSpeed(serverInfo, preflightConfigDownload, preSpeed, [](bool success){
        //if(verbose) std::cout << (success ? '.' : '*') << std::flush;
    })){
        std::cerr << "Pre-flight check failed." << std::endl;
        return EXIT_FAILURE;
    }
    if(verbose) std::cout << std::endl;

    TestConfig uploadConfig   = slowConfigUpload;
    TestConfig downloadConfig = slowConfigDownload;
    if (preSpeed <= 4){
		std::cout << "detected_line_type " << "slowband" << std::endl; 
    } else if (preSpeed > 4 && preSpeed <= 30){
		std::cout << "detected_line_type " << "narrowband" << std::endl; 
        downloadConfig = narrowConfigDownload;
        uploadConfig   = narrowConfigUpload;
    } else if (preSpeed > 30 && preSpeed < 150) {
		std::cout << "detected_line_type " << "broadband" << std::endl; 
        downloadConfig = broadbandConfigDownload;
        uploadConfig   = broadbandConfigUpload;
    } else if (preSpeed >= 150) {
		std::cout << "detected_line_type " << "fiber" << std::endl; 
        downloadConfig = fiberConfigDownload;
        uploadConfig   = fiberConfigUpload;
    }

    if (!upload_only){
        if(verbose) {
			std::cout << std::endl;
        	std::cout << "Testing download speed (" << downloadConfig.concurrency << ") "  << std::flush;
		}
        double downloadSpeed = 0;
        if (sp.downloadSpeed(serverInfo, downloadConfig, downloadSpeed, [](bool success){
            //if(verbose) std::cout << (success ? '.' : '*') << std::flush;
        })){
            if(verbose) std::cout << std::endl;
            std::cout << "download_mbits " << downloadSpeed << std::endl;
        } else {
            std::cerr << "Download test failed." << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (download_only)
        return EXIT_SUCCESS;


    if(verbose) std::cout << "Testing upload speed (" << uploadConfig.concurrency << ") "  << std::flush;
    double uploadSpeed = 0;
    if (sp.uploadSpeed(serverInfo, uploadConfig, uploadSpeed, [](bool success){
        //if(verbose) std::cout << (success ? '.' : '*') << std::flush;
    })){
        if(verbose) std::cout << std::endl;
        std::cout << "upload_mbits " << uploadSpeed << std::endl;
    } else {
        std::cerr << "Upload test failed." << std::endl;
        return EXIT_FAILURE;
    }


    if (share){
        std::string share_it;
        if (sp.share(serverInfo, share_it)){
            std::cout << "results_image_url " << share_it << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
