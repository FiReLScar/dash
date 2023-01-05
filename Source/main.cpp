#include <iostream>
#include <link>
#include <fstream>
#include <vector>
#include <map>
#include <zstr.hpp>

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

int main() {
  std::ifstream keys("Keys");
  std::string key;
  while (std::getline(keys, key)) {
    if (key.find("=") == std::string::npos) continue;
    std::string name = key.substr(0, key.find("="));
    std::string value = key.substr(key.find("=") + 1);
    keyList[name] = value;
  }
  Link Server(3000);
  Server.Error(404, [](Request* req, Response* res) {
    res->Send("404 Not Found");
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

  Server.Get("/css/index.css", [](Request* req, Response* res) {
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "text/css; charset=UTF-8");
    std::ifstream file("www/css/index.css");
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

  Server.Get("/api/data", [](Request* req, Response* res) {
    if (!validate(req, res)) return;
    res->SetHeader("Content-Encoding", "gzip");
    res->SetHeader("Content-Type", "application/json; charset=UTF-8");
    std::ifstream file("bank-api/data.json");
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    res->Send(compress(str));
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
    res->SetHeader("Set-Cookie", "key="+key+"; Path=/");
    res->SetStatus("303 See Other");
    res->SetHeader("Location", "/");
    res->Send("");
  });


  Server.Start();
  return 0;
}