/*
 * userManager.h
 *
 *  Created on: Aug 7, 2018
 *      Author: hhdang
 */

#ifndef APPLICATIONS_SYSTEM_MANAGER_USERMANAGER_H_
#define APPLICATIONS_SYSTEM_MANAGER_USERMANAGER_H_

#include <string>
#include <map>

#include "ini.h"
#include "user.h"

namespace app
{

class userManager
{
private:
    userManager();
    static userManager* s_instance;

    std::map<std::string, app::user> users;
    app::ini userConf;

    void initDefaultUsers();

    void createUser(app::user &user);
    void changeUserPass(app::user &user);
public:
    virtual ~userManager();

    static userManager* getInstance();

    bool initFromFile();
    bool writeToFile();

    std::list< app::user> getUsers();
};

} /* namespace app */

#endif /* APPLICATIONS_SYSTEM_MANAGER_USERMANAGER_H_ */
