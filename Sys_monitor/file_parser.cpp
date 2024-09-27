#include "header.h"

std::vector<ConfigEntry> readConfigFile(const std::string& filePath) {
    std::ifstream configFile(filePath);
    std::vector<ConfigEntry> entries;

    if (!configFile.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier de configuration !" << std::endl;
        return entries;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        ConfigEntry entry;
        std::istringstream iss(line);

        // Lecture de chaque champ s�par� par un ';'
        std::getline(iss, entry.testID, ';');
        std::getline(iss, entry.ctrlName, ';');
        std::getline(iss, entry.ctrlDesc, ';');
        std::getline(iss, entry.params, ';');   // Param�tres sp�cifiques au test
        std::getline(iss, entry.alertType);     // Le type d'alerte

        // Ajout de l'entr�e dans le vecteur
        entries.push_back(entry);
    }

    configFile.close();  // Ne pas oublier de fermer le fichier apr�s lecture
    return entries;
}