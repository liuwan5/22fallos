#include <bits/stdint-uintn.h>
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
#include <sys/types.h>
#include <vector>

/*
  目录项
*/
struct file_entry {
  const char *name;         //文件名
  uint8_t attribute;        //文件属性
  uint32_t cluster_number;  //文件第一个簇号，约定根目录区为0
  uint32_t file_size;       //文件大小
  std::vector<file_entry *> child_files;  //子文件
  std::vector<file_entry *> child_dirs;   //子目录
  bool dot_flag;  //标记该目录项下是否有'.'和'..'目录
  file_entry(const char *n, uint8_t attr, uint32_t clus, uint32_t fs)
      : name(n),
        attribute(attr),
        cluster_number(clus),
        file_size(fs),
        dot_flag(false) {}
};

struct BPB {
  uint16_t BytsPerSec;  //每扇区字节数
  uint8_t SecPerClus;  //每簇扇区数
  uint16_t RsvdSecCnt;  // Boot记录占用的扇区数
  uint8_t NumFATs;     // FAT表个数
  uint16_t RootEntCnt;  //根目录最大文件数
  uint16_t FATSz;       // FAT扇区数
};

const int SECTOR_SIZE = 512;  //扇区大小，单位为字节
char buf[SECTOR_SIZE];        //缓冲区，最大读取一个扇区
short fat_entry[10000];       // fat_entry,存放fat项
std::ifstream image;
BPB  BPB_record; //BPB 全局唯一

void read_fat();
extern "C" void my_print(const char *str);
std::vector<file_entry> *read_file_entries(std::ifstream &image, size_t pos);
int ls(bool lflag, const char *path, file_entry *root_entry);
file_entry *find(const char *path, file_entry *root_entry);
void cat(const char *path, file_entry *root_entry);
const char *parse_path(const char *original_path);
inline int cal_pos_from_cluster_number(int cluster_number) {
  return (1 + 9 + 9 + 14 + cluster_number - 2) * SECTOR_SIZE;
}
int handle_command(file_entry *root_entry);
bool validate_path(const char *path);
file_entry *gen_root_entry() {
  file_entry *root_entry = new file_entry("", 0x10, 0, 0);
  return root_entry;
}
void gen_file_tree(file_entry *root) {  //传入目录项，生成文件树
  std::vector<file_entry> *root_child_entries = read_file_entries(
      image, cal_pos_from_cluster_number(root->cluster_number));
  for (auto &entry : *root_child_entries) {
    if (entry.attribute == 0x20) {
      root->child_files.push_back(&entry);
    }
    if (entry.attribute == 0x10) {
      root->child_dirs.push_back(&entry);
      if (entry.name[0] == '.') {
        root->dot_flag = true;
      } else {
        gen_file_tree(&entry);
      }
    }
  }
}

void read_BPB(){
  image.seekg(0, std::ios::beg);
  image.read(buf, SECTOR_SIZE);
  BPB_record.BytsPerSec = *(uint16_t*)(buf+11);
  BPB_record.SecPerClus = *(uint8_t*)(buf+13);
  BPB_record.RsvdSecCnt = *(uint16_t*)(buf+14);
  BPB_record.NumFATs = *(uint8_t*)(buf+16);
  BPB_record.RootEntCnt = *(uint16_t*)(buf+17);
  BPB_record.FATSz = *(uint16_t*)(buf+19);
}

int main() {
  image.open("./a.img", std::ios::binary | std::ios::in);
  image.seekg(SECTOR_SIZE, std::ios::cur);  //跳过boot区域
  read_fat();
  file_entry *root = gen_root_entry();
  gen_file_tree(root);
  while (handle_command(root))
    ;
  return 0;
}

bool validate_path(const char *path) {
  if (strlen(path) == 0)
    return true;
  else {
    if (path[0] != '/') return false;
  }
  return true;
}

int handle_command(file_entry *root_entry) {
  my_print(">");
  std::string command_line;
  getline(std::cin, command_line);
  std::vector<std::string> commands;
  bool flag = true;  //标记当前字符前是否为空格，初始认为前面是空格
  for (int i = 0; i < command_line.length(); i++) {
    if (command_line[i] != ' ') {
      if (flag) {
        flag = false;
        commands.push_back("");
      }
      commands[commands.size() - 1] += command_line[i];
    } else {
      if (!flag) flag = true;
    }
  }
  std::string target_path;
  if (commands[0] == "ls") {
    bool lflag = false;
    bool path_flag = false;
    for (int i = 1; i < commands.size(); i++) {
      if (commands[i][0] == '-') {
        for (int j = 1; i < commands[j].size(); j++) {
          if (commands[i][j] == 'l') {
            lflag = true;
          } else {
            my_print("Error: unrecognized argument\n");
            return -1;
          }
        }

      } else {
        if (path_flag) {
          my_print("Error: more than 1 path\n");
          return -1;
        } else {
          target_path = commands[i];
        }
      }
    }
    ls(lflag, target_path.empty() ? "" : parse_path(target_path.c_str()),
       root_entry);
  } else {
    if (commands[0] == "cat") {
      if (commands.size() != 2) {
        my_print("Arguments count error!\n");
        return -1;
      } else {
        if (!validate_path(commands[1].c_str())) {
          my_print("invalid path!\n");
          return -1;
        } else {
          cat(parse_path(commands[1].c_str()), root_entry);
        }
      }
    } else if (commands[0] == "exit") {
      if (commands.size() == 1)
        return 0;
      else {
        my_print("Arguments count error!\n");
        return -1;
      }
    } else {
      my_print("Unknown command!\n");
      return -1;
    }
  }
  return 1;
}

/*
  读取fat表项中的内容
*/
void read_fat() {
  const int FAT_SIZE = 9;
  for (int i = 0; i < (FAT_SIZE * SECTOR_SIZE * 2 / 3); i += 2) {
    image.read(buf, 3);
    buf[4] = 0;
    uint32_t tem = *(uint32_t *)buf;
    fat_entry[i] = tem & 0xfff;
    fat_entry[i + 1] = tem >> 12;
  }
}

file_entry *find(const char *path,
                 file_entry *root_entry) {  //传入目标路径和根目录项，返回目标项
  if (strlen(path) == 0) return root_entry;
  if (root_entry->child_dirs.empty() && root_entry->child_files.empty())
    return nullptr;
  const char *p = path;
  std::string tem_name;
  p++;
  while (*p != '/' && *p != '\0') {
    tem_name += *p;
    p++;
  }
  for (auto file : root_entry->child_files) {
    if (tem_name == std::string(file->name)) return file;
  }
  for (auto dir : root_entry->child_dirs) {
    if (tem_name == std::string(dir->name)) return find(p, dir);
  }
  return nullptr;
}

int ls(bool lflag, const char *path, file_entry *root_entry) {
  const char *p = path;
  char *out_str = new char[100];
  std::vector<file_entry> *entries;
  file_entry *target = nullptr;
  if (strlen(path) > 0) {
    target = find(path, root_entry);  //找到当前目标
    if (target == nullptr) {
      my_print("Target not found!\n");
      return -1;
    }
    if (target->attribute != 0x10) {
      my_print("Target is not a dir!\n");
      return -1;
    }
  } else {
    target = root_entry;
  }
  if (!lflag)
    sprintf(out_str, "%s/:\n", path);
  else {
    int child_dir_size = target->child_dirs.size();
    if (target->dot_flag) child_dir_size -= 2;
    sprintf(out_str, "%s/ %d %ld:\n", path, child_dir_size,
            target->child_files.size());
  }
  my_print(out_str);
  for (auto child_dir : target->child_dirs) {
    if (!lflag)
      sprintf(out_str, "\033[31m%s\033[0m  ", child_dir->name);
    else {
      if (child_dir->name[0] == '.') {
        sprintf(out_str, "\033[31m%s\033[0m\n", child_dir->name);
      } else {
        sprintf(out_str, "\033[31m%s\033[0m %ld %ld\n", child_dir->name,
                child_dir->dot_flag ? child_dir->child_dirs.size() - 2
                                    : child_dir->child_dirs.size(),
                child_dir->child_files.size());
      }
    }
    my_print(out_str);
  }
  for (auto file : target->child_files) {
    if (!lflag)
      sprintf(out_str, "\033[0m%s  ", file->name);
    else
      sprintf(out_str, "\033[0m%s %d\n", file->name, file->file_size);
    my_print(out_str);
  }
  if (!lflag) my_print("\n");
  delete[] out_str;
  for (auto child : target->child_dirs) {
    if (child->name[0] == '.') continue;
    std::string root_path = std::string(path);
    std::string child_name = root_path + '/' + std::string(child->name);
    ls(lflag, child_name.c_str(), root_entry);  //递归遍历每个子目录
  }

  return 0;
}

std::vector<file_entry> *read_file_entries(
    std::ifstream &image,
    size_t pos) {  //根据给出的在磁盘中的位置，读取一扇区大小的目录项
  if (pos == cal_pos_from_cluster_number(0)) {
    image.seekg((1 + 9 + 9) * SECTOR_SIZE, std::ios::beg);
  } else {
    image.seekg(pos, std::ios::beg);
  }
  std::vector<file_entry> *entries = new std::vector<file_entry>();
  while (true) {
    image.read(buf, 32);
    if (buf[0] == 0) break;
    char *const name = new char[12];
    char *ptr = name;
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
      ptr++;
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
    entries->emplace_back(name, *(uint8_t *)(buf + 11),
                          (*(uint16_t *)(buf + 26)), *(uint32_t *)(buf + 28));
  }
  return entries;
}

void cat(const char *path, file_entry *root_entry) {
  file_entry *target = find(path, root_entry);
  if (target == nullptr) {
    my_print("File not found!\n");
    return;
  }
  if (target->attribute != 0x20) {
    my_print("Target is not a file!\n");
    return;
  }
  int clus_no = target->cluster_number;
  if (clus_no == 0) return;
  std::string tem_char(2, '\0');
  while (clus_no < 0xFF8) {
    if (clus_no == 0xFF7) {
      my_print("File is broken!\n");
      break;
    }
    image.seekg(cal_pos_from_cluster_number(clus_no), std::ios::beg);
    image.read(buf, SECTOR_SIZE);

    for (char c : buf) {
      tem_char[0] = c;
      tem_char[1] = 0;
      my_print(tem_char.c_str());
    }

    clus_no = fat_entry[clus_no];
  }
  tem_char[0] = '\n';
  my_print(tem_char.c_str());
}

const char *parse_path(
    const char *original_path) {  //对路径进行处理，去掉路径中的'.'和'..'
  std::vector<std::string> directories;  //存放当前已解析出的目录的栈
  const char *p = original_path;
  if (*p == '/') p++;
  while (*p != 0) {
    std::string dir_name;
    while (*p != '/' && *p != 0) {
      dir_name += *p;
      p++;
    }
    if (*p != 0) p++;
    if (dir_name == "")
      continue;
    else if (dir_name == ".")
      continue;
    else if (dir_name == "..") {
      if (directories.empty())
        continue;
      else {
        directories.pop_back();
      }
    } else {
      directories.emplace_back(dir_name);
    }
  }
  std::string *result = new std::string("");
  for (std::string &str : directories) {
    *result += '/';
    *result += str;
  }
  return result->c_str();
}