#include "header.h"
using namespace std;

void network_checker_wrapper(const std::string& adapterName, const std::string& logFileName) {
    network_checker(adapterName, logFileName);  // On appelle la vraie fonction, mais on ignore le r�sultat
}

// Wrapper pour list_removable_drives qui ajoute un deuxi�me param�tre sans l'utiliser
void list_removable_drives_wrapper(const std::string& params, const std::string& logFileName) {
    list_removable_drives(logFileName);  // On appelle la vraie fonction sans utiliser 'params'
}

// Fonction pour ex�cuter la fonction bas�e sur l'ID
void executeTest(const std::string& testID, const std::string& params, int warningThreshold, int alertThreshold, const std::string& logFileName) {
    // Map pour les fonctions n�cessitant plusieurs param�tres (wchar_t*, int, int, string)
    std::map<std::string, std::function<void(const wchar_t*, int, int, const std::string&)>> testMapWithMultipleParams = {
        { "001", disk_free_space }
    };

    // Map pour les fonctions n�cessitant un seul param�tre (params + logFileName)
    std::map<std::string, std::function<void(const std::string&, const std::string&)>> testMapWithParams = {
        { "002", network_checker_wrapper },  // Utilisation du wrapper
        { "003", list_removable_drives_wrapper }  // Utilisation du wrapper
    };

    // Map pour les fonctions sans param�tre (seulement logFileName)
    std::map<std::string, std::function<void(const std::string&)>> testMapNoParams = {
        { "004", ListRunningServices }
    };

    // Appel des fonctions avec plusieurs param�tres
    auto itWithMultipleParams = testMapWithMultipleParams.find(testID);
    if (itWithMultipleParams != testMapWithMultipleParams.end()) {
        std::stringstream ss(params);
        std::string driveLetter;
        std::getline(ss, driveLetter, ',');

        // Conversion de std::string en std::wstring pour Windows API
        std::wstring wDriveLetter(driveLetter.begin(), driveLetter.end());

        std::string warningStr, alertStr;
        std::getline(ss, warningStr, ',');
        std::getline(ss, alertStr, ',');

        int parsedWarningThreshold = std::stoi(warningStr);
        int parsedAlertThreshold = std::stoi(alertStr);

        itWithMultipleParams->second(wDriveLetter.c_str(), parsedWarningThreshold, parsedAlertThreshold, logFileName);
        return;
    }

    // Appel des fonctions avec un seul param�tre
    auto itWithParams = testMapWithParams.find(testID);
    if (itWithParams != testMapWithParams.end()) {
        itWithParams->second(params, logFileName);
        return;
    }

    // Appel des fonctions sans param�tre
    auto itNoParams = testMapNoParams.find(testID);
    if (itNoParams != testMapNoParams.end()) {
        itNoParams->second(logFileName);
        return;
    }

    // Si l'ID du test n'est pas trouv�
    logToFile("ID du test non trouv� : " + testID, logFileName);
}

void executeTestsFromConfig(const std::string& configFile, int warningThreshold, int alertThreshold, const std::string& logFileName) {
    // Lire le fichier de configuration
    std::vector<ConfigEntry> configEntries = readConfigFile(configFile);

    // Pour chaque entr�e dans le fichier config, ex�cuter le test correspondant
    for (const auto& entry : configEntries) {
        logToFile("Ex�cution du test : " + entry.testID + " (" + entry.ctrlName + ")", logFileName);

        // Appeler la fonction en fonction de l'ID du test et des param�tres
        executeTest(entry.testID, entry.params, warningThreshold, alertThreshold, logFileName);
    }
}

/*int main() {
    int warningThreshold = 80;
    int alertThreshold = 95;

    std::string configFile = "config.txt";
    std::string logFileName = "system_monitor_log.txt";

    logMachineInfo("system_monitor_log.txt");
    executeTestsFromConfig(configFile, warningThreshold, alertThreshold, logFileName);

    return 0;
}*/

/*int main() {
    try {
        // Initialiser le driver MySQL et la connexion
        sql::mysql::MySQL_Driver* driver;
        sql::Connection* con;

        // Cr�er une instance du driver et se connecter � MySQL
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "root", "Mah010505!");

        // Cr�er un statement pour ex�cuter les requ�tes SQL
        sql::Statement* stmt = con->createStatement();

        // Cr�er une nouvelle base de donn�es
        stmt->execute("CREATE DATABASE IF NOT EXISTS sys_monitor");

        // Afficher un message de confirmation
        std::cout << "Base de donn�es 'sys_monitor' cr��e avec succ�s." << std::endl;

        // Fermer les ressources
        delete stmt;
        delete con;

    }
    catch (sql::SQLException& e) {
        std::cerr << "Erreur SQL: " << e.what() << std::endl;
    }

    return 0;
}*/

int main() {
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;

    // Cr�e la connexion
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "root", "Mah010505!");

    // S�lectionne la base de donn�es
    con->setSchema("sys_monitor");

    delete con;
    return 0;
}
