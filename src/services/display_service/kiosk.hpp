#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <curl/curl.h>
#include <opencv2/opencv.hpp>
#include "../../libraries/json.hpp"

#include "lvgl.h"
#include "pillo_logo.h"
#include "banner.h"

namespace display{
    namespace main_menu{
        // Callback function to write response data to a buffer
        size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
            size_t datasize = size * nmemb;
            // Cast and append data to your buffer
            std::string *buffer = static_cast<std::string*>(stream);
            buffer->append((char*)ptr, datasize);
            return datasize;
        }

        // Function to cURL and fetch json data
        std::string getJson(std::string str){
            std::string url = "https://positive-clearly-tiger.ngrok-free.app/verify?encoding_string=";
            url.append(str);

            std::cout << "target url: " << url << std::endl;

            CURLcode ret;
            CURL *hnd;
            std::string response_buffer; // Initialize an empty string to store the response

            hnd = curl_easy_init();
            if (hnd) {
                curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
                curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
                curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
                curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.2.1");
                curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
                curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
                curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
                curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

                // Set the write callback function and the buffer pointer
                curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data);
                curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response_buffer);

                ret = curl_easy_perform(hnd);

                if (ret == CURLE_OK) {
                    // Success! Access the received data in response_buffer
                    //std::cout << "Response data:\n" << response_buffer << std::endl;
                } else {
                    std::cout << "curl error: " << curl_easy_strerror(ret) << std::endl;
                }

                curl_easy_cleanup(hnd);
            }

            return response_buffer;
        }

        // Function to open video device and scan a QR code and return the string
        std::string ScanAndFetch(){
            std::string qrData;

            // Open the video capture device
            cv::VideoCapture cap("/dev/video0");
            
            // Check if the video capture device opened successfully
            if (!cap.isOpened()) {
                std::cout << "Error opening video capture device" << std::endl;
                return "";
            }

            // Create a QRCodeDetector object
            cv::QRCodeDetector qrDecoder;

            while (true) {
                cv::Mat frame;

                cap >> frame;

                // Check if the frame is empty
                if (frame.empty()) {
                    std::cout << "End in video stream !!!" << std::endl;
                    break;
                }

                // Detect and decode the QR code
                qrData = qrDecoder.detectAndDecode(frame);

                // Check if the QR code is detected and decoded successfully
                if (qrData.empty()) {
                    //std::cout << "QR code not detected or could not be decoded" << std::endl;
                } else {
                    //std::cout << "Decoded data: " << qrData << std::endl;
                    break;
                }

                cv::waitKey(16);
            }

            cap.release();

            return qrData;
        }

        static void event_handler(lv_event_t * e)
        {
            lv_event_code_t code = lv_event_get_code(e);
            if(code == LV_EVENT_CLICKED) {
                exit(0);
            }
        }
        
        void menu_app(){
            lv_obj_t * qrbanner = lv_img_create(lv_scr_act());
            lv_img_dsc_t banner;
            banner.header.cf = LV_IMG_CF_TRUE_COLOR;
            banner.header.always_zero = 0;
            banner.header.reserved = 0;
            banner.header.w = 512;
            banner.header.h = 512;
            banner.data_size = 262144 * LV_COLOR_SIZE / 8;
            banner.data = banner_map;

            lv_img_set_src(qrbanner, &banner);
            lv_obj_align(qrbanner, LV_ALIGN_CENTER, 0, 0);

            lv_obj_t* order_text = lv_label_create(lv_scr_act());
            lv_obj_set_style_text_font(order_text, &lv_font_montserrat_32, 0);
            lv_obj_set_align(order_text, LV_ALIGN_CENTER);
            lv_label_set_text(order_text, "");

            for(int i = 0; i < 100; i++) {
                lv_task_handler();
                usleep(5000);
            }

            lv_task_handler();

            std::string data = ScanAndFetch();
            std::string response = getJson(data);

            std::cout << "response json: " << response << std::endl;

            nlohmann::json output = nlohmann::json::parse(response);
            
            lv_obj_del(qrbanner);
    
            if(output["verified"] == false){
                lv_obj_set_style_text_color(order_text, lv_color_hex(0xd85431), LV_PART_MAIN);
                lv_label_set_text(order_text, "Invalid prescription!");
                for(int i = 0; i < 500; i++) {
                    lv_task_handler();
                    usleep(5000);
                }
                return;
            }
            else{
                lv_obj_set_style_text_color(order_text, lv_color_hex(0x3e03ff), LV_PART_MAIN);
                
                lv_label_set_text_fmt(order_text, "Prescription ID: %d", (int)output["prescription"]["prescriptionID"]);
            }

            for(int i = 0; i < 500; i++) {
                lv_task_handler();
                usleep(5000);
            }

            nlohmann::json medicines = output["prescription"]["medicines"];
            int n = 1;
            for (const auto& med : medicines) {
                lv_label_set_text_fmt(order_text, "Packing %d of %d: %s",  n, (int)output["prescription"]["medsNo"], 
                                                                                    std::string(med["medName"].get<std::string>()).c_str());

                for(int i = 0; i < 500; i++) {
                    lv_task_handler();
                    usleep(5000);
                }
            }

            lv_label_set_text(order_text, "Please collect your medicines from the counter");


            for(int i = 0; i < 500; i++) {
                lv_task_handler();
                usleep(5000);
            }

            lv_obj_del(order_text);
            lv_task_handler();
        }
    }
}

