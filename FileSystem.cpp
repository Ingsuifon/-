#include "FileSystem.h"
using namespace std;

FileSystem::FileSystem() {
    userList["usr1"] = "123456";
    userList["usr2"] = "123456";
    userList["usr3"] = "123456";
    codes[0] = "logout";
    codes[1] = "create";
    codes[2] = "open";
    codes[3] = "close";
    codes[4] = "write";
    codes[5] = "read";
    codes[6] = "del";
    codes[7] = "mkdir";
    codes[8] = "rmdir";
    codes[9] = "dir";
    codes[10] = "cd";
    codes[11] = "format";
    fd = -1;
    currentDir = new char[DIR_LENGTH];
    fdisk = nullptr;
}

bool FileSystem::login(string& user, string& pwd) {
    bool res = userList[user] == pwd;
    if (res) {
        this->user = user;
        strcpy(currentDir, (user + ":").c_str());
    }
    return res;
}

int FileSystem::create(char* name) {
    if (strlen(name) > 15)
        return -1;
    int i, j;
    /*找一个空闲的子目录（文件）表项*/
    for (i = 2; i < MSD + 2; i++) {
        if (cur_dir->direct_item[i].first == -1)
            break;
    }
    /*检查是否重名*/
    for (j = 2; j < MSD + 2; j++) {
        if (!strcmp(cur_dir->direct_item[j].name, name) && !cur_dir->direct_item[j].isDir)
            break;
    }
    /*无空闲子目录（文件）项*/
    if (i == MSD + 2)
        return -2;
    /*当前打开文件数已满*/
    if (u_opentable.cur_size >= FILE_OPEN_NUM)
        return -3;
    /*文件重名*/
    if (j < MSD + 2)
        return -4;
    for (j = ROOT_BLOCK_NO + 3; j < BLOCK_NUM; j++) {
        if (fat[j].is_free)
            break;
    }
    /*没有空闲块*/
    if (j == BLOCK_NUM)
        return -5;
    fat[j].is_free = false;
    strcpy(cur_dir->direct_item[i].name, name);
    cur_dir->direct_item[i].first = j;
    cur_dir->direct_item[i].size = 0;
    cur_dir->direct_item[i].isDir = false;
    fd = open(name);
    return 0;
}

int FileSystem::open(char *fileName) {
    int i, j;
    for (i = 2; i < MSD + 2; i++) {
        if (!strcmp(cur_dir->direct_item[i].name, fileName) && !cur_dir->direct_item[i].isDir)
            break;
    }
    /*文件不存在*/
    if (i == MSD + 2)
        return -1;
    for (auto & item: u_opentable.openitem) {
        if (!strcmp(item.name, fileName))
            return -2;
    }
    if (u_opentable.cur_size >= FILE_OPEN_NUM)
        return -3;
    for (j = 0; j < FILE_OPEN_NUM; j++) {
        if (u_opentable.openitem[j].first == -1)
            break;
    }
    u_opentable.openitem[j].first = cur_dir->direct_item[i].first;
    strcpy(u_opentable.openitem[j].name, fileName);
    u_opentable.openitem[j].size = cur_dir->direct_item[i].size;
    u_opentable.cur_size++;
    return j;
}

void FileSystem::run() {
    string usr, pwd, ins;
    cout << "username: ";
    getline(cin, usr);
    cout << "password: ";
    getline(cin, pwd);
    if (!login(usr, pwd)) {
        cout << "Mistake! " << endl;
        return;
    }

    fstream fs;
    char ch, name[10], *context;
    context = new char[MAX_WRITE];
    int i, flag, r_size;
    fs.open("disk.dat", ios::binary | ios::in);
    if (!fs) {
        cout << "You have not format, Do you want format?(y/n)" << endl;
        cin >> ch;
        if (ch == 'y' || ch == 'Y') {
            init_file();
            cout << "Successfully format!" << endl;
        }
        else
            return;
    }

    enter();
    print();
    showCurrentDir();

    while (true) {
        cin >> ins;
        int num;
        for (num = 0; num < 12; num++)
            if (codes[num] == ins)
                break;
        switch (num) {
            case 0:
                delete[] context;
                return;
            case 1:  /*创建文件*/
                scanf("%s", name);
                flag = create(name);
                if (flag == -1)
                    cout << "Error: length is too long !" << endl;
                else if (flag == -2)
                    cout << "Error: The direct item is already full !" << endl;
                else if (flag == -3)
                    cout << "Error: The number of openfile is too much !" << endl;
                else if (flag == -4)
                    cout << "Error: The name is already in the direct !" << endl;
                else if (flag == -5)
                    cout << "Error: The disk space is full!" << endl;
                else
                    cout << "Successfully create a file!" << endl;
                showCurrentDir();
                break;
            case 2:  /*打开文件*/
                scanf("%s", name);
                fd = open(name);
                if (fd == -1)
                    cout << "Error: The open file not exit!" << endl;
                else if (fd == -2)
                    cout << "Error: The file have already opened!" << endl;
                else if (fd == -3)
                    cout << "Error: The number of open file is too much!" << endl;
                else if (fd == -4)
                    cout << "Error: It is a direct,can not open for read or write!" << endl;
                else
                    cout << "Successfully opened!" << endl;
                showCurrentDir();
                break;
            case 3:  /*关闭文件*/
                scanf("%s", name);
                flag = close(name);
                if (flag == -1)
                    cout << "Error: The file is not opened !" << endl;
                else
                    cout << "Successfully closed" << endl;
                showCurrentDir();
                break;
            case 4:  /*写文件*/
                if (fd == -1)
                    cout << "Error: The file is not opened!" << endl;
                else {
                    cout << "Please input the file context: ";
                    scanf("%s", context);
                    flag = write(context, strlen(context));
                    if (flag)
                        cout << "Error! " << endl;
                    else
                        cout << "Successfully write!" << endl;
                }
                showCurrentDir();
                break;
            case 5:  /*读文件*/
                if (fd == -1)
                    cout << "Error: The file is not opened!" << endl;
                else {
                    flag = read(context);
                    if (flag)
                        cout << "Error!" << endl;
                    else {
                        for (i = 0; i < u_opentable.openitem[fd].size; i++)
                            cout << context[i];
                        cout << "\t\n";
                    }
                }
                showCurrentDir();
                break;
            case 6:  /*删除文件*/
                scanf("%s", name);
                flag = del(name);
                if (flag == -1)
                    cout << "Error: The file not exit!" << endl;
                else if (flag == -2)
                    cout << "Error: The file is opened,please first close it !" << endl;
                else if (flag == -3)
                    cout << "Error: The delete is not file !" << endl;
                else
                    cout << "Successfully delete!" << endl;
                showCurrentDir();
                break;
            case 7:    /*创建子目录*/
                scanf("%s", name);
                flag = mkdir(name);
                if (flag == -1)
                    cout << "Error: The length of name is to long!" << endl;
                else if (flag == -2)
                    cout << "Error: The direct item is already full !" << endl;
                else if (flag == -3)
                    cout << "Error: The name is already in the direct !" << endl;
                else if (flag == -4)
                    cout << "Error: can not in the name of a direct !" << endl;
                else if (flag == -5)
                    cout << "Error: The disk space is full!" << endl;
                else if (flag == -6)
                    cout << "Error: '..' or '.' can not as the name of the direct!" << endl;
                else
                    cout << "Successfully make direct!" << endl;
                showCurrentDir();
                break;
            case 8:    /*删除子目录*/
                scanf("%s", name);
                flag = rmdir(name);
                if (flag == -1)
                    cout << "Error: The direct is not exist!" << endl;
                else if (flag == -2)
                    cout << "Error: The direct has son direct, please first remove the son direct!" << endl;
                else if (flag == -3)
                    cout << "Error: The remove is not direct !" << endl;
                else
                    cout << "Successfully remove direct!" << endl;
                showCurrentDir();
                break;
            case 9:    /*展示目录*/
                dir();
                showCurrentDir();
                break;
            case 10:   /*更改目录*/
                scanf("%s", name);
                flag = cd(name);
                if (flag == -1)
                    cout << "Error: The path no correct!" << endl;
                else if (flag == -2)
                    cout << "Error: The '/' is too much !" << endl;
                showCurrentDir();
                break;
            case 11:   /*格式化*/
                cout << "WARNING!!! Do you want format?(y/n)" << endl;
                cin >> ch;
                if (ch == 'y' || ch == 'Y') {
                    for (auto& item: u_opentable.openitem) {
                        strcpy(item.name, "");
                        item.first = -1;
                        item.size = 0;
                    }
                    u_opentable.cur_size = 0;
                    cur_dir = root;
                    format();
                    root = cur_dir;
                    strcpy(currentDir, (user + ":").c_str());
                    cout << "Successfully format!" << endl;
                }
                showCurrentDir();
                break;
            default:
                cout << "No such command!" << endl;
                showCurrentDir();
                break;
        }
    }
}

void FileSystem::showCurrentDir() {
    printf("%s> ", currentDir);
}

FileSystem::~FileSystem() {
    if (fdisk != nullptr) {
        fstream fs;
        fs.open("disk.dat", ios::out | ios::binary);
        fs.write(fdisk, MEM);
        fs.close();
    }
    delete[] fdisk;
    delete[] currentDir;
}

void FileSystem::init_file() {
    fdisk = new char[MEM];
    format();
    delete[] fdisk;
}

void FileSystem::format() {
    int i;
    fstream fs;   /*文件流对象*/
    fat = reinterpret_cast<FAT*>(fdisk);
    for (i = 0; i < ROOT_BLOCK_NO - 1; i++) {
        fat[i].item = i + 1;
        fat[i].is_free = false;
    }
    fat[ROOT_BLOCK_NO - 1].item = -1;
    fat[ROOT_BLOCK_NO - 1].is_free = false;
    fat[ROOT_BLOCK_NO].item = -1;
    fat[ROOT_BLOCK_NO].is_free = false;
    fat[ROOT_BLOCK_NO + 1].item = -1;
    fat[ROOT_BLOCK_NO + 1].is_free = false;
    fat[ROOT_BLOCK_NO + 2].item = -1;
    fat[ROOT_BLOCK_NO + 2].is_free = false;
    for (i = ROOT_BLOCK_NO + 3; i < BLOCK_NUM; i++) {
        fat[i].item = -1;
        fat[i].is_free = true;
    }

    /*用户1*/
    root = reinterpret_cast<Direct*>(fdisk + (ROOT_BLOCK_NO) * BLOCK_SIZE);   /*根目录地址*/
    /*初始化目录项*/
    /*指向当前目录的目录项*/
    root->direct_item[0].isRoot = true;
    root->direct_item[0].first = ROOT_BLOCK_NO;
    strcpy(root->direct_item[0].name, ".");
    root->direct_item[0].isDir = true;
    root->direct_item[0].size = ROOT_BLOCK_SIZE;
    /*指向上一级目录的目录项*/
    root->direct_item[1].isRoot = true;
    root->direct_item[1].first = ROOT_BLOCK_NO;
    strcpy(root->direct_item[1].name, "..");
    root->direct_item[1].isDir = true;
    root->direct_item[1].size = ROOT_BLOCK_SIZE;
    /*初始化子目录（文件）项*/
    for (i = 2; i < MSD + 2; i++) {
        root->direct_item[i].isRoot = false;
        root->direct_item[i].first = -1;
        strcpy(root->direct_item[i].name, "");
        root->direct_item[i].isDir = false;
        root->direct_item[i].size = 0;
    }

    /*用户2*/
    root = reinterpret_cast<Direct*>(fdisk + (ROOT_BLOCK_NO + 1) * BLOCK_SIZE);   /*根目录地址*/
    /*初始化目录项*/
    /*指向当前目录的目录项*/
    root->direct_item[0].isRoot = true;
    root->direct_item[0].first = ROOT_BLOCK_NO + 1;
    strcpy(root->direct_item[0].name, ".");
    root->direct_item[0].isDir = true;
    root->direct_item[0].size = ROOT_BLOCK_SIZE;
    /*指向上一级目录的目录项*/
    root->direct_item[1].isRoot = true;
    root->direct_item[1].first = ROOT_BLOCK_NO + 1;
    strcpy(root->direct_item[1].name, "..");
    root->direct_item[1].isDir = true;
    root->direct_item[1].size = ROOT_BLOCK_SIZE;
    /*初始化子目录（文件）项*/
    for (i = 2; i < MSD + 2; i++) {
        root->direct_item[i].isRoot = false;
        root->direct_item[i].first = -1;
        strcpy(root->direct_item[i].name, "");
        root->direct_item[i].isDir = false;
        root->direct_item[i].size = 0;
    }

    /*用户3*/
    root = reinterpret_cast<Direct*>(fdisk + (ROOT_BLOCK_NO + 2 ) * BLOCK_SIZE);   /*根目录地址*/
    /*初始化目录项*/
    /*指向当前目录的目录项*/
    root->direct_item[0].isRoot = true;
    root->direct_item[0].first = ROOT_BLOCK_NO + 2;
    strcpy(root->direct_item[0].name, ".");
    root->direct_item[0].isDir = true;
    root->direct_item[0].size = ROOT_BLOCK_SIZE;
    /*指向上一级目录的目录项*/
    root->direct_item[1].isRoot = true;
    root->direct_item[1].first = ROOT_BLOCK_NO + 2;
    strcpy(root->direct_item[1].name, "..");
    root->direct_item[1].isDir = true;
    root->direct_item[1].size = ROOT_BLOCK_SIZE;
    /*初始化子目录（文件）项*/
    for (i = 2; i < MSD + 2; i++) {
        root->direct_item[i].isRoot = false;
        root->direct_item[i].first = -1;
        strcpy(root->direct_item[i].name, "");
        root->direct_item[i].isDir = false;
        root->direct_item[i].size = 0;
    }
    fs.open("disk.dat", ios::out | ios::binary);
    fs.write(fdisk, MEM);
    fs.close();
}

int FileSystem::close(char* fileName) {
    int i;
    for (i = 0; i < FILE_OPEN_NUM; i++) {
        if (!strcmp(u_opentable.openitem[i].name, fileName))
            break;
    }
    if (i == FILE_OPEN_NUM)
        return -1;
    /*清空打开文件表项*/
    u_opentable.openitem[i].first = -1;
    strcpy(u_opentable.openitem[i].name, "");
    u_opentable.openitem[i].size = 0;
    u_opentable.cur_size--;
    fd = -1;
    return 0;
}

int FileSystem::write(char *buff, size_file len) {
    char* first;
    int i, j, temp;
    int item;
    size_file ilen1, ilen2;
    /*用$作为空格，#作为换行符*/
    for (i = 0; i < len; i++) {
        if (buff[i] == '$')
            buff[i] = ' ';
        else if (buff[i] == '#')
            buff[i] = '\n';
    }
    /*用户打开表对应表项第一个盘块号*/
    item = u_opentable.openitem[fd].first;
    for (i = 2; i < MSD + 2; i++) {
        if (cur_dir->direct_item[i].first == item)
            break;
    }
    temp = i;    /*存放当前目录项的下标*/

    while (fat[item].item != -1)
        item = fat[item].item;
    /*找到文件的最末尾处*/
    first = fdisk + item * BLOCK_SIZE + u_opentable.openitem[fd].size % BLOCK_SIZE;
    /*如果最后的磁盘块可以写入全部数据*/
    if (BLOCK_SIZE - u_opentable.openitem[fd].size % BLOCK_SIZE > len) {
        strcpy(first, buff);
        u_opentable.openitem[fd].size += len;
        cur_dir->direct_item[temp].size += len;
    }
    else {
        size_file n = BLOCK_SIZE - u_opentable.openitem[fd].size % BLOCK_SIZE;
        strncpy(first, buff, n);
//        for (i = 0; i < (BLOCK_SIZE - u_opentable.openitem[fd].size % BLOCK_SIZE); i++)
//            first[i] = buff[i];
        buff += n;
        ilen1 = len - n;
        ilen2 = ilen1 / BLOCK_SIZE;
        ilen2 += ilen1 % BLOCK_SIZE == 0 ? 0 : 1;
        for (j = 0; j < ilen2; j++) {
            for (i = ROOT_BLOCK_NO + 3; i < BLOCK_NUM; i++) {
                if (fat[i].is_free)
                    break;
            }
            if (i == BLOCK_NUM)
                return -1;
            first = fdisk + i * BLOCK_SIZE;
            /*写的最后一块磁盘块*/
            if (j != ilen2 - 1) {
//                for (int k = 0; k < BLOCK_SIZE; k++)
//                    first[k] = buff[k];
                strncpy(first, buff, ilen1);
            }
            else {
//                for (int k = 0; k < len - (BLOCK_SIZE - u_opentable.openitem[fd].size % BLOCK_SIZE) - j * BLOCK_SIZE; k++)
//                    first[k] = buff[k];
                strncpy(first, buff, BLOCK_SIZE);
                ilen1 -= BLOCK_SIZE;
            }
            buff += BLOCK_SIZE;
            fat[item].item = i;
            fat[i].is_free = false;
            fat[i].item = -1;
            item = i;
        }
        u_opentable.openitem[fd].size += len;
        cur_dir->direct_item[temp].size += len;
    }
    return 0;
}

int FileSystem::read(char *buff) {
    size_file len = u_opentable.openitem[fd].size;
    char* first;
    int ilen1;
    int item = u_opentable.openitem[fd].first;
    ilen1 = len / BLOCK_SIZE;
    ilen1 += len % BLOCK_SIZE == 0 ? 0 : 1;
    first = fdisk + item * BLOCK_SIZE;
    for (int i = 0; i < ilen1; i++) {
        if (i == ilen1 - 1)
            strncpy(buff + i * BLOCK_SIZE, first, len - i * BLOCK_SIZE);
        else
            strncpy(buff + i * BLOCK_SIZE, first, BLOCK_SIZE);
        item = fat[item].item;
        first = fdisk + item * BLOCK_SIZE;
    }
    return 0;
}

int FileSystem::del(char* fileName) {
    int i;
    int cur_item, item, temp;
    for (i = 2; i < MSD + 2; i++)
        if (!strcmp(cur_dir->direct_item[i].name, fileName))
            break;
    cur_item = i;
    if (i == MSD + 2)
        return -1;
    if (cur_dir->direct_item[cur_item].isDir)
        return -3;
    for (i = 0; i < FILE_OPEN_NUM; i++)
        if (!strcmp(u_opentable.openitem[i].name, fileName))
            return -2;
    item = cur_dir->direct_item[cur_item].first;
    /*释放磁盘块*/
    while (item != -1) {
        temp = fat[item].item;
        fat[item].item = -1;
        fat[item].is_free = true;
        item = temp;
    }
    /*删除目录项*/
    Direct::FCB& fcb = cur_dir->direct_item[cur_item];
    fcb.isRoot = false;
    fcb.first = -1;
    strcpy(fcb.name, "");
    fcb.isDir = false;
    fcb.size = 0;
    return 0;
}

int FileSystem::mkdir(char *fileName) {
    int i, j;
    Direct* cur_mkdir;
    if (strchr(fileName, '/'))
        return -4;
    if (!strcmp(fileName, "."))
        return -6;
    if (!strcmp(fileName, ".."))
        return -6;
    if (strlen(fileName) > 8)
        return -1;
    for (i = 2; i < MSD + 2; i++) {
        if (cur_dir->direct_item[i].first == -1)
            break;
    }
    /*无空闲目录（文件）项*/
    if (i == MSD + 2)
        return -2;
    for (j = 2; j < MSD + 2; j++) {
        if (!strcmp(cur_dir->direct_item[j].name, fileName))
            break;
    }
    if (j < MSD + 2)
        return -3;
    for (j = ROOT_BLOCK_NO + 3; j < BLOCK_NUM; j++) {
        if (fat[j].is_free)
            break;
    }
    if (j == BLOCK_NUM)
        return -5;
    fat[j].is_free = false;
    /*填写目录项*/
    Direct::FCB& fcb = cur_dir->direct_item[i];
    strcpy(fcb.name, fileName);
    fcb.first = j;
    fcb.size = BLOCK_SIZE;
    fcb.isDir = true;
    cur_mkdir = reinterpret_cast<Direct*>(fdisk + fcb.first * BLOCK_SIZE);
    /*初始化目录*/
    /*指向当前目录的目录项*/
    Direct::FCB& cur1 = cur_mkdir->direct_item[0];
    cur1.isRoot = false;
    cur1.first = fcb.first;
    strcpy(cur1.name, ".");
    cur1.isDir = true;
    cur1.size = ROOT_BLOCK_SIZE;
    /*指向上一级目录*/
    Direct::FCB& cur2 = cur_mkdir->direct_item[1];
    cur2.isRoot = cur_dir->direct_item[0].isRoot;
    cur2.first = cur_dir->direct_item[0].first;
    strcpy(cur2.name, "..");
    cur2.isDir = true;
    cur2.size = ROOT_BLOCK_SIZE;
    /*初始化子目录项*/
    for (i = 2; i < MSD + 2; i++) {
        Direct::FCB& tmp = cur_mkdir->direct_item[i];
        tmp.isRoot = false;
        tmp.first = -1;
        strcpy(tmp.name, "");
        tmp.isDir = false;
        tmp.size = 0;
    }
    return 0;
}

int FileSystem::rmdir(char *fileName) {
    int i, j;
    Direct* temp_dir;
    for (i = 2; i < MSD + 2; i++) {
        if (!strcmp(cur_dir->direct_item[i].name, fileName))
            break;
    }
    Direct::FCB& fcb = cur_dir->direct_item[i];
    if (!fcb.isDir)
        return -3;
    if (i == MSD + 2)
        return -1;
    temp_dir = reinterpret_cast<Direct*>(fdisk + fcb.first * BLOCK_SIZE);
    for (j = 2; j < MSD + 2; j++) {
        if (temp_dir->direct_item[j].first != -1) {
            Direct* tmp = cur_dir;
            cur_dir = temp_dir;
            if (temp_dir->direct_item[j].isDir) {
                char* tmp_name = new char[DIR_LENGTH];
                strcpy(tmp_name, temp_dir->direct_item[j].name);
                rmdir(tmp_name);
                delete[] tmp_name;
            }
            else
                del(temp_dir->direct_item[j].name);
            cur_dir = tmp;
        }
    }
    int item = fcb.first;
    fat[item].is_free = true;
    /*修改目录项*/
    fcb.isRoot = false;
    fcb.isDir = false;
    strcpy(fcb.name, "");
    fcb.first = -1;
    fcb.size = 0;
    return 0;
}

void FileSystem::dir() {
    for (auto& fcb: cur_dir->direct_item) {
        if (fcb.first != -1) {
            printf("%s\t", fcb.name);
            if (!fcb.isDir)
                printf("%d\t\t\n", fcb.size);
            else
                printf("\t<DIR>\t\n");
        }
    }
}

int FileSystem::cd(char* name) {
    int i, j, item;
    char *str, *str1, *temp, *point, *point1;
    Direct* temp_dir = cur_dir;
    str = name;
    if (!strcmp("/", name)) {
        cur_dir = root;
        strcpy(currentDir, (user + ":").c_str());
        return 0;
    }
    j = 0;
    for (i = 0; i < static_cast<int>(strlen(str)); i++) {
        if (name[i] == '/') {
            j++;
            if (j >= 2)
                return -2;    /*太多连续的斜杠*/
        }
        else
            j = 0;
    }
    if (name[0] == '/') {    /*绝对路径*/
        temp_dir = root;
        strcpy(currentDir, (user + ":").c_str());
        str++;
    }
    if (str[strlen(str) - 1] == '/')
        str[strlen(str) - 1] = '\0';
    str1 = strchr(str, '/');             /*字符/的位置*/
    temp = new char[DIR_LENGTH];
    while (str1 != nullptr) {
        for (i = 0; i < str1 - str; i++)
            temp[i] = str[i];
        temp[i] = '\0';                     /*字符/前面的目录名*/
        for (j = 0; j < MSD + 2; j++) {
            if (!strcmp(temp_dir->direct_item[j].name, temp) && temp_dir->direct_item[j].isDir)  /*查找子目录是否含有这个目录*/
                break;
        }
        if (j == MSD + 2)
            return -1;
        item = temp_dir->direct_item[j].first;
        temp_dir = reinterpret_cast<Direct*>(fdisk + item * BLOCK_SIZE);
        str = str1 + 1;
        str1 = strchr(str, '/');
    }
    str1 += strlen(str);
    for (i = 0; i < static_cast<int>(strlen(str)); i++)
        temp[i] = str[i];
    temp[i] = '\0';
    for (j = 0; j < MSD + 2; j++) {
        if (!strcmp(temp_dir->direct_item[j].name, temp) && temp_dir->direct_item[j].isDir)  /*查找子目录是否含有这个目录*/
            break;
    }
    delete[] temp;
    if (j == MSD + 2)
        return -1;
    item = temp_dir->direct_item[j].first;
    temp_dir = reinterpret_cast<Direct*>(fdisk + item * BLOCK_SIZE);
    if (!strcmp("..", name) || !strcmp("..", str)) {
        if (!cur_dir->direct_item[j - 1].isRoot) {
            point = strchr(currentDir, '/');
            while (point != nullptr) {
                point1 = point + 1;
                point = strchr(point1, '/');
            }
            *(point1 - 1) = '\0';     /*例如：usr/haha --> usr\0haha相当于截断后面的目录*/
        }
    }
    else if (!strcmp(".", name))
        ;
    else {
        if (name[0] != '/')
            currentDir = strcat(currentDir, "/");
        currentDir = strcat(currentDir, name);
    }
    cur_dir = temp_dir;
    return 0;
}

void FileSystem::print() {
    cout << "********************************************************************************" << endl;
    cout << "\t\t\tWelcome to NEU File System!" << endl;
    cout << "--------------------------------------------------------------------------------" << endl;
    cout << "\t\t 退出系统  logout" << endl;
    cout << "\t\t 创建文件  create 文件名" << endl;
    cout << "\t\t 删除文件  del 文件名" << endl;
    cout << "\t\t 打开文件  open 文件名" << endl;
    cout << "\t\t 关闭文件  close 文件名" << endl;
    cout << "\t\t 写文件   write" << endl;
    cout << "\t\t 读文件   read" << endl;
    cout << "\t\t 创建目录  mkdir 目录名" << endl;
    cout << "\t\t 删除子目录  rmdir 目录名" << endl;
    cout << "\t\t 显示当前目录的子目录 dir" << endl;
    cout << "\t\t 更改当前目录  cd 目录名" << endl;
    cout << "\t\t 格式化  format" << endl;
}

void FileSystem::enter() {
    fstream fs;
    fdisk = new char[MEM];    /*申请1M内存*/
    fs.open("disk.dat", ios::in | ios::binary);
    if (!fs) {
        cout << "Error: Cannot open disk file!" << endl;
        exit(1);
    }
    fs.read(fdisk, MEM);
    if (user == "usr1")
        root = reinterpret_cast<Direct*>(fdisk + (ROOT_BLOCK_NO) * BLOCK_SIZE);
    else if (user == "usr2")
        root = reinterpret_cast<Direct*>(fdisk + (ROOT_BLOCK_NO + 1) * BLOCK_SIZE);
    else if (user == "usr3")
        root = reinterpret_cast<Direct*>(fdisk + (ROOT_BLOCK_NO + 2) * BLOCK_SIZE);
    fat = reinterpret_cast<FAT*>(fdisk);
    fs.close();
    /*初始化用户打开表*/
    for (auto& i : u_opentable.openitem) {
        strcpy(i.name, "");
        i.first = -1;
        i.size = 0;
    }
    u_opentable.cur_size = 0;
    cur_dir = root;    /*当前目录设为根目录*/
}