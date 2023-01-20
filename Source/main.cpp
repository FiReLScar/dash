#include <iostream>
#include <link>
#include <fstream>
#include <vector>
#include <map>
#include <zstr.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

std::vector<std::string> split(std::string data, std::string delimiter) {
  std::vector<std::string> result;
  size_t pos = 0;
  std::string token;
  while ((pos = data.find(delimiter)) != std::string::npos) {
    token = data.substr(0, pos);
    if (token != "") result.push_back(token);
    data.erase(0, pos + delimiter.length());
  }
  if (data != "") result.push_back(data);
  return result;
}

std::string replace(std::string data, std::string delimiter, std::string replacement) {
  size_t pos = 0;
  while ((pos = data.find(delimiter)) != std::string::npos) {
    data.replace(pos, delimiter.length(), replacement);
  }
  return data;
}

std::string getCookie(std::string data, std::string name) {
  std::vector<std::string> cookies = split(data, "; ");
  std::string c = "";
  for (std::string cookie: cookies) {
    if (cookie[cookie.length()-1] == '\r') cookie = cookie.substr(0, cookie.length()-1);
    if (cookie.substr(0,name.length()+1) == name+"=") {
      c = cookie.substr(name.length()+1);
      break;
    }
  }
  return c;
}

std::map<std::string, std::string> keyList;

std::string compress(std::string data) {
  std::ostringstream output;
  std::istringstream ds(data);

  zstr::ostream zs(output);
  const std::streamsize size = 1 << 16;
  char* buf = new char[size];
  while (true) {
    ds.read(buf, size);
    std::streamsize count = ds.gcount();
    if (count == 0) break;
    zs.write(buf, count);
  }
  delete [] buf;
  return (std::ostringstream&)zs << std::flush, output.str();
}

bool validate(Request* req, Response* res) {
  std::string key = getCookie(req->GetHeader("cookie"), "key");
  if (keyList.find(key) == keyList.end()) {
    res->SetHeader("Set-Cookie", "key=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/html; charset=UTF-8");
    std::ifstream file("www/invalidkey.html");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
    return false;
  }
  return true;
}

void SendMessageToLights(std::string message) {
  int sock = 0, valread, client_fd;
  struct sockaddr_in serv_addr;
  char buffer[1024] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cout << "Bro how do you not have permission to start a client?" << std::endl;
    return;
  }
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(1337);
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
    std::cout << "Are you stupid? Did you even start the server?" << std::endl;
    return;
  }
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cout << "Failed to connect to Lights API" << std::endl;
    return;
  }
  send(sock, message.c_str(), message.length(), 0);
  close(sock);
}

bool isFloat(std::string myString) {
    std::istringstream iss(myString);
    float f;
    iss >> std::noskipws >> f;
    return iss.eof() && !iss.fail(); 
}

int main() {
  std::ifstream keys("Keys");
  std::string key;
  while (std::getline(keys, key)) {
    if (key.find("=") == std::string::npos) continue;
    std::string name = key.substr(0, key.find("="));
    std::string value = key.substr(key.find("=") + 1);
    keyList[name] = value;
  }

  // Bills DB
  {std::ifstream BillsDB("Bills");
  if (!BillsDB.is_open()) {
    std::ofstream bills("Bills");
    bills.close();
  }
  BillsDB.close();}

  Link Server(3000);
  Server.Error(404, [](Request* req, Response* res) {
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/html; charset=UTF-8");
    std::ifstream file("www/404.html");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
  });
  Server.Default([](Request* req, Response* res) {
    res->Error(404);
  });

  Server.Get("/", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    std::string key = getCookie(req->GetHeader("cookie"), "key");
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/html; charset=UTF-8");
    std::ifstream file("www/index.html");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    str = replace(str, "[name]", keyList[key]);
    res->Send(compress(str));
  });

  Server.Get("/iot", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    std::string key = getCookie(req->GetHeader("cookie"), "key");
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/html; charset=UTF-8");
    std::ifstream file("www/iot.html");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    str = replace(str, "[name]", keyList[key]);
    res->Send(compress(str));
  });

  Server.Get("/bills", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    std::string key = getCookie(req->GetHeader("cookie"), "key");
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/html; charset=UTF-8");
    std::ifstream file("www/bills.html");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    str = replace(str, "[name]", keyList[key]);
    res->Send(compress(str));
  });

  Server.Get("/css/index.css", [](Request* req, Response* res) {
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/css; charset=UTF-8");
    std::ifstream file("www/css/index.css");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
  });

  Server.Get("/css/global.css", [](Request* req, Response* res) {
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/css; charset=UTF-8");
    std::ifstream file("www/css/global.css");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
  });

  Server.Get("/css/navbar.css", [](Request* req, Response* res) {
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/css; charset=UTF-8");
    std::ifstream file("www/css/navbar.css");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
  });

  Server.Get("/js/bank.js", [](Request* req, Response* res) {
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/javascript; charset=UTF-8");
    std::ifstream file("www/js/bank.js");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
  });

  Server.Get("/js/bills.js", [](Request* req, Response* res) {
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/javascript; charset=UTF-8");
    std::ifstream file("www/js/bills.js");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
  });

  Server.Get("/js/mobile.js", [](Request* req, Response* res) {
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/javascript; charset=UTF-8");
    std::ifstream file("www/js/mobile.js");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
  });

  Server.Get("/api/data", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "application/json; charset=UTF-8");
    std::ifstream file("bank-api/data.json");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
  });

  Server.Get("/api/iot/{data}", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    std::string data = req->GetQuery("data");
    if (data == "on") {
      SendMessageToLights("On");
    } else if (data == "off") {
      SendMessageToLights("Off");
    } else if (data.substr(0, 5) == "Color") {
      if (data.substr(6).find(",") == std::string::npos) {
        res->SetHeader("Content-Encoding", "gzip");
        res->SetHeader("Content-Type", "application/json; charset=UTF-8");
        res->Send(compress("{\"status\": \"error\"}"));
        return;
      }
      std::vector<std::string> hsv = split(data.substr(6), ",");
      if (hsv.size() != 3) {
        res->SetHeader("Content-Encoding", "gzip");
        res->SetHeader("Content-Type", "application/json; charset=UTF-8");
        res->Send(compress("{\"status\": \"error\"}"));
        return;
      }

      for (int i = 0; i < 3; i++) {
        if (!isFloat(hsv[i])) {
          res->SetHeader("Content-Encoding", "gzip");
          res->SetHeader("Content-Type", "application/json; charset=UTF-8");
          res->Send(compress("{\"status\": \"error\"}"));
          return;
        }
      }
      SendMessageToLights("Color: " + hsv[0] + "," + hsv[1] + "," + hsv[2]);
    } else {
      res->SetHeader("Content-Encoding", "gzip");
      res->SetHeader("Content-Type", "application/json; charset=UTF-8");
      res->Send(compress("{\"status\": \"error\"}"));
      return;
    }
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "application/json; charset=UTF-8");
    res->Send(compress("{\"status\": \"ok\"}"));
  });

  Server.Get("/api/bills/add", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    std::string name = req->GetQuery("name");
    std::string amount = req->GetQuery("amount");
    std::string frequency = req->GetQuery("frequency");
    std::string started = req->GetQuery("started");

    if (name == "" || amount == "" || frequency == "" || started == "" || !isFloat(amount) || !isFloat(started) || name.find(",") != std::string::npos) {
      res->Send("error");
      return;
    }

    if (frequency != "m" && frequency != "y" && frequency != "w" && frequency != "b") {
      res->Send("error");
      return;
    }

    std::ifstream read("Bills");
    std::string line;
    while (std::getline(read, line)) {
      std::vector<std::string> data = split(line, ",");
      if (data[0] == name) {
        res->Send("error");
        return;
      }
    }
    read.close();
    std::ofstream write("Bills", std::ios_base::app);
    write << name << "," << amount << "," << frequency << "," << started << std::endl;
    write.close();
    res->Send("ok");
  });

  Server.Get("/api/bills/edit", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    std::string name = req->GetQuery("name");
    std::string amount = req->GetQuery("amount");
    std::string frequency = req->GetQuery("frequency");
    std::string started = req->GetQuery("started");

    if (name == "" || amount == "" || frequency == "" || started == "" || !isFloat(amount) || !isFloat(started) || name.find(",") != std::string::npos) {
      res->Send("error");
      return;
    }

    if (frequency != "m" && frequency != "y" && frequency != "w" && frequency != "b") {
      res->Send("error");
      return;
    }

    std::ifstream read("Bills");
    std::string line;
    std::string replacement = "";
    bool found = false;
    while (std::getline(read, line)) {
      std::vector<std::string> data = split(line, ",");
      if (data[0] == name) {
        found = true;
        replacement += data[0] + "," + amount + "," + frequency + "," + started + "\n";
        continue;
      }
    }
    if (!found) {
      res->Send("error");
      return;
    }
    read.close();
    std::ofstream write("Bills");
    write << replacement;
    write.close();
    res->Send("ok");
  });

  Server.Get("/api/bills/remove", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    std::string name = req->GetQuery("name");

    if (name == "" || name.find(",") != std::string::npos) {
      res->Send("error");
      return;
    }

    std::ifstream read("Bills");
    std::string line;
    std::string replacement = "";
    bool found = false;
    while (std::getline(read, line)) {
      std::vector<std::string> data = split(line, ",");
      if (data[0] == name) {
        found = true;
        continue;
      }
      replacement += line + "\n";
    }
    read.close();
    if (!found) {
      res->Send("error");
      return;
    }
    std::ofstream write("Bills");
    write << replacement;
    write.close();
    res->Send("ok");
  });

  Server.Get("/api/bills/get", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    std::ifstream read("Bills");
    std::string data((std::istreambuf_iterator<char>(read)), std::istreambuf_iterator<char>());
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/text; charset=UTF-8");
    res->Send(compress(data));
  });

  Server.Get("/profile.png", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    std::string key = getCookie(req->GetHeader("cookie"), "key");
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "image/png");
    std::ifstream file("www/pfp/"+key+".png");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
  });

  Server.Get("/setkey/{key}", [](Request* req, Response* res) {
    std::string key = req->GetQuery("key");
    if (keyList.find(key) == keyList.end()) {
      res->SetHeader("Content-Encoding", "gzip");
      res->SetHeader("Content-Type", "text/html; charset=UTF-8");
      std::ifstream file("www/invalidkey.html");
      std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      res->Send(compress(str));
      return;
    }
    res->SetHeader("Set-Cookie", "key="+key+"; Path=/; Expires=Thu, 01 Jan 2030 00:00:00 GMT");
    res->SetStatus("303 See Other");
    res->SetHeader("Location", "/");
    res->Send("");
  });


  Server.Start();
  return 0;
}