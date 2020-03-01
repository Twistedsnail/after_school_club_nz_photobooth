#include "curl_manager.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <curl/curl.h>
#include <Magick++.h>

#define SENDER      "nathan.wride@gmail.com"
#define RECIPIENT   "anahitapaulphoto@gmail.com"

static std::string test_payload =
    "To: <" RECIPIENT ">\r\n"
    "From: <" SENDER ">\r\n"
    "Subject: Big Dawg Photos\r\n"
    "Content-type: multipart/mixed; boundary=\"simple boundary\"\r\n"
    "--simple boundary\r\n"
    "Here's some juicy shit.\r\n"
    "Comment, like and subscribe.\r\n"
    "\r\n";

static void add_img_attachment(std::string name, std::string path) {
    Magick::Image img(path.c_str());
    Magick::Blob blob;
    img.write(&blob);
    std::string payload = blob.base64();

    test_payload += "--simple boundary\r\n";
    test_payload += "Content-Type: image/jpeg; name=\"";
    test_payload += name;
    test_payload += "\"\r\n";
    test_payload += "Content-Transfer-Encoding: base64\r\n";
    test_payload += "Content-Disposition: attachment; filename=\"";
    test_payload += name;
    test_payload += "\"\r\n\r\n";
    test_payload += payload;
    test_payload += "\r\n";
}

static size_t payload_source(void *ptr, size_t sze, size_t nmemb, void *userp) {
    if(sze == 0 || nmemb == 0 || sze * nmemb < 1) return 0;

    unsigned *bytes_read = (unsigned *)userp;
    unsigned read_size = sze*nmemb;

    if(test_payload.size() == *bytes_read) return 0;
    else if(test_payload.size() < *bytes_read + read_size) {
        read_size = test_payload.size() - *bytes_read;
    }

    std::string data_portion = test_payload.substr(*bytes_read, sze*nmemb);
    const char *data = data_portion.c_str();
    if(data) {
        *bytes_read += read_size;
        size_t len = strlen(data);
        printf("Sending payload bytes %i of %i\n", *bytes_read, test_payload.size());
        memcpy(ptr, data, len);

        return len;
    }

    return 0;
}

void curl_test() {
    add_img_attachment("capture1.jpg", "../data/capture1.jpg");
    add_img_attachment("capture2.jpg", "../data/capture2.jpg");
    printf("Loaded both\n");

    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    unsigned bytes_read = 0;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, "nathan.wride@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "Uw0ntg3tt415!");
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<" SENDER ">");
        recipients = curl_slist_append(recipients, "<" RECIPIENT ">");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &bytes_read);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);
        if(res == !CURLE_OK) {
            printf("ERROR: %s\n", curl_easy_strerror(res));
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
}
