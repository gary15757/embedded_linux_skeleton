/*
 * userManager.cpp
 *
 *  Created on: Aug 7, 2018
 *      Author: hhdang
 */
#include <iostream>
#include <list>
#include <syslog.h>

#include "userManager.h"

namespace app
{

userManager::userManager()
{
    // TODO Auto-generated constructor stub
}

userManager::~userManager()
{
    // TODO Auto-generated destructor stub
}

userManager *userManager::s_instance = 0;

userManager* userManager::getInstance()
{
    if (s_instance == 0) {
        s_instance = new userManager();
    }

    return s_instance;
}

void userManager::initDefaultUsers()
{
    this->users.clear();

    FILE *fp;

    if ((fp = fopen("/etc/passwd", "w")) == NULL) {
        syslog(LOG_EMERG, "Cannot add default users to linux\n");
        return;
    }

    fprintf(fp,
        "root:x:0:0:root:/root:/bin/sh\n"
        "daemon:x:1:1:daemon:/usr/sbin:/bin/false\n"
        "bin:x:2:2:bin:/bin:/bin/false\n"
        "sys:x:3:3:sys:/dev:/bin/false\n"
        "sync:x:4:100:sync:/bin:/bin/sync\n"
        "mail:x:8:8:mail:/var/spool/mail:/bin/false\n"
        "www-data:x:33:33:www-data:/var/www:/bin/false\n"
        "operator:x:37:37:Operator:/var:/bin/false\n"
        "nobody:x:65534:65534:nobody:/home:/bin/false\n"
    );

    fclose(fp);

    std::list<app::user> defaultUsers;
    app::user user;
    user.setName("root");
    user.setFullName("root");
    user.setPassword("root");
    defaultUsers.push_back(user);

    user.setName("admin");
    user.setFullName("admin");
    user.setPassword("admin");
    defaultUsers.push_back(user);

    for(auto &u : defaultUsers) {
        this->addUser(u);
    }
}

void userManager::createUser(app::user &user)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "adduser "
            "-h /home " // set home dir
            "-s /bin/sh "
            "-H "// Don't create home dir
            "-D "// Don't assign a password
            "-G "// add user to group users
            "users "
            "%s >/dev/null 2>&1", user.getName().c_str());

    system(cmd);
}

void userManager::changeUserPass(app::user &user)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "echo -e \"%s\\n%s\" | passwd %s >/dev/null 2>&1",
            user.getPassword().c_str(),
            user.getPassword().c_str(),
            user.getName().c_str());

    system(cmd);
}

bool userManager::addUser(app::user &user)
{
    bool rc = false;

    if (user.isValid()) {

        auto it = this->users.find(user.getName());

        if (it == this->users.end()) {
            this->users.insert(std::pair<std::string, app::user>(user.getName(), user));
            this->createUser(user);
        } else {
            it->second = user;
        }

        this->changeUserPass(user);

        syslog(LOG_NOTICE, "add user %s to the user list", user.getName().c_str());

        rc = true;
    }

    return rc;
}

void userManager::initFromFile()
{
    initDefaultUsers();

    app::ini userConf;

    if (userConf.loadFromFile("/data/users.conf")) {

        for(int i = 0; i < 50; i++) {

            char sect[32];
            std::string value;

            snprintf(sect, sizeof(sect), "user_%d", i);
            if (userConf.get_string(sect, "name", value)) {

                syslog(LOG_NOTICE, "found user %s from /data/users.conf", value.c_str());

                app::user user;
                user.setName(value.c_str());

                if (userConf.get_string(sect, "password", value)) {
                    user.setPassword(value.c_str());
                }

                if (userConf.get_string(sect, "fullname", value)) {
                    user.setFullName(value.c_str());
                }

                if (userConf.get_string(sect, "email", value)) {
                    user.setEmail(value.c_str());
                }

                this->addUser(user);
            }
        }

    } else {
        syslog(LOG_NOTICE, "cannot load user config from /data/users.conf, use default users");
    }

    if (this->writeToFile() != true) {
        syslog(LOG_WARNING, "cannot write user config to /data/users.conf");
    }
}

bool userManager::writeToFile()
{
    app::ini userConf;
    int i = 0;
    for (auto it = this->users.begin(); it != this->users.end(); it++)
    {
        char sect[32];
        std::string value;

        snprintf(sect, sizeof(sect), "user_%d", i++);

        value = it->second.getName();
        userConf.set_string(sect, "name", value);

        value = it->second.getPassword();
        userConf.set_string(sect, "password", value);

        value = it->second.getFullName();
        userConf.set_string(sect, "fullname", value);

        value = it->second.getEmail();
        userConf.set_string(sect, "email", value);
    }

    return userConf.writeToFile("/data/users.conf");
}

void userManager::getUsers(std::list<app::user> &users)
{
    for (auto it = this->users.begin(); it != this->users.end(); it++)
    {
        users.push_back(it->second);
    }
}

} /* namespace app */
