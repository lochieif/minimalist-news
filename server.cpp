#include<stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <json/json.h> 
#include <sstream>
#include <fstream>

#define PORT 8081
#define WEB_ROOT "./webroot"

const char *NEWS_API_KEY;

void initialize_news_api_key() {
    NEWS_API_KEY = getenv("news_api_key");
    if (NEWS_API_KEY == NULL) {
        fprintf(stderr, "Error: news_api_key environment variable not set.\n");
        exit(1);
            }
}

size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t new_length = size * nmemb;
    s->append((char*)contents, new_length);
    return new_length;
}

std::string fetch_news(std::string category, int is_au) {
  CURL* curl;
  CURLcode res;
  std::string read_buffer;
  
  curl = curl_easy_init();
  
    if (curl) {
        std::string url = "https://newsapi.org/v2/top-headlines?";
        if (is_au == 0) {
        url.append("country=au");
        } else {
        url.append("language=en");
        }
        url.append("&category=");    
        url.append(category);
        url.append("&apiKey=");

        url.append(NEWS_API_KEY);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "User-Agent: MyMinimalNewsAggregator/1.0");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers); // free the header list

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
    }


    return read_buffer;


}

std::string fetch_todays_news() {
  printf("FETCHING\n");
  CURL* curl;
  CURLcode res;
  std::string read_buffer;
  curl = curl_easy_init();
  
    if (curl) {
        std::string url = "https://newsapi.org/v2/top-headlines?country=au&apiKey=";
        url.append(NEWS_API_KEY);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "User-Agent: MyMinimalNewsAggregator/1.0");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers); // free the header list

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
    }


    return read_buffer;
  }

void handle_news(int client_socket, std::string category, int is_au) {
   printf("handling news\n");
    std::string news_data = fetch_news(category, is_au);

    Json::CharReaderBuilder reader_builder;
    Json::Value json_data;
    std::string errs;

    std::istringstream s(news_data);
    if (Json::parseFromStream(reader_builder, s, &json_data, &errs)) {
        std::cout << "parsing json" << "\n";
        if (json_data.isMember("articles")) {
            std::ostringstream response;

            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: text/html\r\n\r\n";
            response << "<html><body>\n";

            const Json::Value articles = json_data["articles"];
            for (const auto &article : articles) {
                std::string title = article["title"].asString();
                std::string url = article["url"].asString();

                // Format as HTML link and append to response
                response << "<a href='" << url << "' target='_blank'>" << title << "</a><br>\n";
            }

            response << "</body></html>\n";

            // Convert response to string and send to client
            std::string response_str = response.str();
            send(client_socket, response_str.c_str(), response_str.size(), 0);
        } else {
            std::string error_response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            send(client_socket, error_response.c_str(), error_response.size(), 0);
        }
    } else {
        std::string error_response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        send(client_socket, error_response.c_str(), error_response.size(), 0);
    }

}

/*void handle_business_news(int client_socket, std::string category) {
    printf("handling news\n");
    std::string news_data = fetch_aus_business_news();

    Json::CharReaderBuilder reader_builder;
    Json::Value json_data;
    std::string errs;

    std::istringstream s(news_data);
    if (Json::parseFromStream(reader_builder, s, &json_data, &errs)) {
        std::cout << "parsing json" << "\n";
        if (json_data.isMember("articles")) {
            std::ostringstream response;

            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: text/html\r\n\r\n";
            response << "<html><body>\n";

            const Json::Value articles = json_data["articles"];
            for (const auto &article : articles) {
                std::string title = article["title"].asString();
                std::string url = article["url"].asString();

                // Format as HTML link and append to response
                response << "<a href='" << url << "' target='_blank'>" << title << "</a><br>\n";
            }

            response << "</body></html>\n";

            // Convert response to string and send to client
            std::string response_str = response.str();
            send(client_socket, response_str.c_str(), response_str.size(), 0);
        } else {
            std::string error_response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            send(client_socket, error_response.c_str(), error_response.size(), 0);
        }
    } else {
        std::string error_response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        send(client_socket, error_response.c_str(), error_response.size(), 0);
    }
}
*/

void handle_daily_news(int client_socket) {
    printf("handling news\n");
    std::string news_data = fetch_todays_news();

    Json::CharReaderBuilder reader_builder;
    Json::Value json_data;
    std::string errs;

    std::istringstream s(news_data);
    if (Json::parseFromStream(reader_builder, s, &json_data, &errs)) {
        std::cout << "parsing json" << "\n";
        if (json_data.isMember("articles")) {
            std::ostringstream response;

            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: text/html\r\n\r\n";
            response << "<html><body>\n";

            const Json::Value articles = json_data["articles"];
            for (const auto &article : articles) {
                std::string title = article["title"].asString();
                std::string url = article["url"].asString();

                // Format as HTML link and append to response
                response << "<a href='" << url << "' target='_blank'>" << title << "</a><br>\n";
            }

            response << "</body></html>\n";

            // Convert response to string and send to client
            std::string response_str = response.str();
            send(client_socket, response_str.c_str(), response_str.size(), 0);
        } else {
            std::string error_response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            send(client_socket, error_response.c_str(), error_response.size(), 0);
        }
    } else {
        std::string error_response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        send(client_socket, error_response.c_str(), error_response.size(), 0);
    }
}

void serve_file(int client_socket, const std::string& file_path) {
    std::ifstream file(WEB_ROOT + file_path, std::ios::in | std::ios::binary);
    std::cout << "serving index.html to client\n";
    if (!file) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return;
    }

    std::ostringstream file_content;
    file_content << file.rdbuf();
    file.close();

    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(file_content.str().size()) + "\r\n\r\n" + file_content.str();
    send(client_socket, response.c_str(), response.size(), 0);
}

// Function to handle dynamic requests
void handle_request(int client_socket, const std::string& request) {
    std::cout << "in_request with " << request;
    if (request.find("/daily_news") != std::string::npos) {
        // Handle /daily_news request (sample response)
        handle_daily_news(client_socket);
        //const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from server!";
        //send(client_socket, response, strlen(response), 0);
    } else if (request.find("/index.html") != std::string::npos || request.find("GET /index.html HTTP/1.1") != std::string::npos) {
        // Serve index.html
        printf("request is index.html GET\n");
        serve_file(client_socket, "/index.html");
    } else if (request.find("/business_australia") != std::string::npos) {
        handle_news(client_socket,"business",0);
      
    } else if (request.find("/international") != std::string::npos) {
      handle_news(client_socket,"",1);
    } else if (request.find("/int_business") != std::string::npos) {
      handle_news(client_socket,"business",1);
    }
    else {
        // Handle other requests or return 404 Not Found
        const char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
        send(client_socket, response, strlen(response), 0);
    }
}



char *parse(char line[], const char symbol[]) {
  char *message;
  char *token;
  
  token = strtok(line,symbol);

  int current = 0;

  while(token != NULL) {
    std::cout << token;
    token = strtok(NULL, " ");
    if (current == 0) {
      message = token;
      return message;

    }
    current = current + 1;
    
  }

  return message; 
}
void start_server() {
  
  int server_fd, new_socket;
  struct sockaddr_in address;
  long valread;
  socklen_t address_len = sizeof(address);

  char http_header[25] = "HTTP/1.1 200 Ok\r\n";

  if ((server_fd = socket(AF_INET,SOCK_STREAM,0)) == 0) {
    perror("In socket");
    exit(EXIT_FAILURE);
  }
  
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);
  
  memset(address.sin_zero, '\0', sizeof address.sin_zero);
  

  if ((bind(server_fd,(struct sockaddr *)&address,address_len)) < 0) {
    perror("In bind");
    //close(server_fd);
    exit(EXIT_FAILURE);
  }

  if ((listen(server_fd, 10)) < 0) {
    perror("In listen");
    exit(EXIT_FAILURE);
  }

    std::cout << "Listening on port 8081\n";

  while (true) {

  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&address_len)) < 0) {
    perror("In accept");
    exit(EXIT_FAILURE);
    }
  std::cout << "recieved connection\n";

  struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&address;
		struct in_addr ipAddr = pV4Addr->sin_addr;
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);  //#include <arpa/inet.h>
    std::cout << "Client IP:" << str << "\n";
  

  char buffer[30000] = { 0 };
  valread = read(new_socket, buffer, 30000);
  printf("read the buffer");
  if (valread < 0) {
    perror("Error reading from socket");
    close(new_socket);
  }
  
  printf("\n%s \n", buffer);

  char *parse_string = parse(buffer," ");
  printf("Client asked for path: %s\n", parse_string);

  handle_request(new_socket, parse_string);

  close(new_socket);
  }
}

int main (int argc, char *argv[]) {
  initialize_news_api_key();
  start_server();
  return 0;
}
