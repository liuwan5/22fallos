#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

/*
  目录项
*/
struct file_entry {
  char* name;               //文件名
  uint8_t attribute;        //文件属性
  uint32_t cluster_number;  //文件第一个簇号
  uint32_t file_size;       //文件大小
  file_entry(char* n, uint8_t attr, uint32_t clus, uint32_t fs)
      : name(n), attribute(attr), cluster_number(clus), file_size(fs) {}
};

const int SECTOR_SIZE = 512;  //扇区大小，单位为字节
char buf[SECTOR_SIZE];        //缓冲区，最大读取一个扇区
short fat_entry[10000];       // fat_entry,存放fat项
std::ifstream image;

void read_fat();
void my_print(const char* str);
std::vector<file_entry>* read_file_entries(std::ifstream& image, size_t pos);
int ls(bool lflag = false, const char* path = "");
int find(const char* path);
void cat(const char* path);

int main() {
  std::string image_name;
  std::cin >> image_name;
  std::cin >> std::ws;
  image.open(image_name, std::ios::binary | std::ios::in);
  image.seekg(SECTOR_SIZE, std::ios::cur);  //跳过boot区域
  read_fat();
  std::string command_line;
  getline(std::cin, command_line);
  std::vector<std::string> commands(0);
  commands.emplace_back("");
  for (int i = 0; i < command_line.length(); i++) {
    if (command_line[i] != ' ') {
      commands[commands.size()] += command_line[i];
    } else {
      commands.push_back("");
    }
  }
  std::string target_path;
  if (commands[0] == "ls") {
    bool lflag = false;
    for(int i=1; i<commands.size(); i++){
      if(commands[i][0]=='-'){
        if(commands[i][1]=='l'){
          lflag = true;
        }
      }else{
        target_path = commands[i];
      }
    }
    ls(lflag, target_path.empty()?"":target_path.c_str());
  } else {
    if (commands[0] == "cat") {
    } else {
      return -1;
    }
  }
  cat("/NJU");
  return 0;
}

/*
  读取fat表项中的内容
*/
void read_fat() {
  const int FAT_SIZE = 9;
  for (int i = 0; i < (FAT_SIZE * SECTOR_SIZE * 2 / 3); i += 2) {
    image.read(buf, 3);
    buf[4] = 0;
    uint32_t tem = *(uint32_t*)buf;
    fat_entry[i] = tem & 0xfff;
    fat_entry[i + 1] = tem >> 12;
  }
}

int find(const char* path) {
  const char* p = path;
  int pos = (1 + 9 + 9) * 512;
  std::vector<file_entry>* entries;
  while (strlen(p) > 1) {  //逐层寻找，找到目标位置
    entries = read_file_entries(image, pos);  //读取当前层的子目录/文件
    std::string dir_name;
    p++;  //跳过'/'
    while (*p != '/' && *p != '\0') {
      dir_name += *p;
      p++;
    }
    for (file_entry& entry : *entries) {
      if (std::string(entry.name) ==
          dir_name) {  //在当前层的子目录/文件中找到目标
        pos = (1 + 9 + 9 + 14 + entry.cluster_number - 2) * 512;
      }
    }
  }
  return pos;
}

int ls(bool lflag, const char* path) {
  const char* p = path;
  int pos = (1 + 9 + 9) * 512;
  char* out_str = new char[100];
  std::vector<file_entry>* entries;
  entries = read_file_entries(image, find(path));  //读取目标目录的子目录、文件
  std::vector<file_entry*> child_dirs;
  std::vector<file_entry*> files;
  for (auto& entry : *entries) {
    if (entry.attribute == 0x20) {
      files.push_back(&entry);
    }
    if (entry.attribute == 0x10) {
      if (entry.name[0] != '.') child_dirs.push_back(&entry);
    }
  }
  if (!lflag)
    sprintf(out_str, "%s/:\n", path);
  else{
    sprintf(out_str, "%s/ %d %d:\n", path, child_dirs.size(), files.size());
  }
  my_print(out_str);
  
  for(auto file:files) {
    if(!lflag)
      sprintf(out_str, "\033[0m%s ", file->name);
    else
      sprintf(out_str, "\033[0m%s %d\n", file->name, file->file_size);
    my_print(out_str);
  }
  for(auto child_dir: child_dirs) {
    if(!lflag)
      sprintf(out_str, "\033[31m%s\033[0m ", child_dir->name);
    else{
      std::vector<file_entry>* child_entries = entries = read_file_entries(image, (1+9+9+14+child_dir->cluster_number-2)*SECTOR_SIZE);
      int file_counter=0, child_dir_counter=0;
      for (auto& entry : *entries) {
        if (entry.attribute == 0x20) {
          file_counter++;
        }
        if (entry.attribute == 0x10) {
          if (entry.name[0] != '.') child_dir_counter++;
        }
      }
      sprintf(out_str, "\033[31m%s\033[0m %d %d\n", child_dir->name, child_dir_counter, file_counter);
    }
    my_print(out_str);
  }
  my_print("\n");
  delete[] out_str;
  for (auto child : child_dirs) {
    std::string root_path = std::string(path);
    std::string child_name = root_path + '/' + std::string(child->name);
    ls(lflag, child_name.c_str());
  }

  return 0;
}

std::vector<file_entry>* read_file_entries(std::ifstream& image, size_t pos) {
  image.seekg(pos, std::ios::beg);
  std::vector<file_entry>* entries = new std::vector<file_entry>();
  while (true) {
    image.read(buf, 32);
    if (buf[0] == 0) break;
    char* const name = new char[12];
    char* ptr = name;
    for (int i = 0; i < 8; i++) {
      if (buf[i] != 0x20) {
        *ptr = buf[i];
        ptr++;
      } else {
        break;
      }
    }
    if (buf[8] != 0x20) {
      *ptr = '.';
      for (int i = 0; i < 3; i++) {
        if (buf[i + 8] != 0) {
          *ptr = buf[i + 8];
          ptr++;
        } else {
          break;
        }
      }
      *ptr = 0;
    }
    entries->emplace_back(name, *(uint8_t*)(buf + 11), (*(uint16_t*)(buf + 26)),
                          *(uint32_t*)(buf + 28));
  }
  return entries;
}

void cat(const char* path) {
  int pos = find(path);
  image.seekg(pos, std::ios::beg);
  image.read(buf, SECTOR_SIZE);
  for (char c : buf) {
    putchar(c);
  }
}