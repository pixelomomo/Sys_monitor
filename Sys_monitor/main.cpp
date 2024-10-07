#include "header.h"
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#pragma warning(disable : 4275)
#pragma execution_character_set("utf-8")
using namespace std;

void network_checker_wrapper(const std::string& adapterName, const std::string& logFileName) {
    network_checker(adapterName, logFileName);
}

void list_removable_drives_wrapper(const std::string& params, const std::string& logFileName) {
    list_removable_drives(logFileName);
}

void executeTest(const std::string& testID, const std::string& params, int warningThreshold, const std::string& logFileName) {
    // Map pour les fonctions nécessitant plusieurs paramètres (wchar_t*, int, string)
    std::map<std::string, std::function<void(const wchar_t*, int, const std::string&)>> testMapWithMultipleParams = {
        { "1", disk_free_space }  // Ne prend désormais qu'un seul seuil (warningThreshold)
    };

    // Map pour les fonctions nécessitant un seul paramètre (params + logFileName)
    std::map<std::string, std::function<void(const std::string&, const std::string&)>> testMapWithParams = {
        { "2", network_checker_wrapper },
        { "3", list_removable_drives_wrapper }
    };

    // Map pour les fonctions sans paramètre (seulement logFileName)
    std::map<std::string, std::function<void(const std::string&)>> testMapNoParams = {
        { "4", ListRunningServices }
    };

    // Appel des fonctions avec plusieurs paramètres (comme pour disk_free_space)
    auto itWithMultipleParams = testMapWithMultipleParams.find(testID);
    if (itWithMultipleParams != testMapWithMultipleParams.end()) {
        std::stringstream ss(params);
        std::string driveLetter;
        std::getline(ss, driveLetter, ',');

        // Conversion de std::string en std::wstring pour Windows API
        std::wstring wDriveLetter(driveLetter.begin(), driveLetter.end());

        // Suppression du paramètre d'alerte, seule la valeur du seuil d'avertissement est utilisée
        itWithMultipleParams->second(wDriveLetter.c_str(), warningThreshold, logFileName);
        return;
    }

    // Appel des fonctions avec un seul paramètre (params + logFileName)
    auto itWithParams = testMapWithParams.find(testID);
    if (itWithParams != testMapWithParams.end()) {
        itWithParams->second(params, logFileName);
        return;
    }

    // Appel des fonctions sans paramètre (juste logFileName)
    auto itNoParams = testMapNoParams.find(testID);
    if (itNoParams != testMapNoParams.end()) {
        itNoParams->second(logFileName);
        return;
    }

    // Si l'ID du test n'est pas trouvé
    logToFile("ID du test non trouvé : " + testID, logFileName);
}

void executeTestsFromConfig(const std::string& configFile, int warningThreshold, const std::string& logFileName) {
    std::vector<ConfigEntry> configEntries = readConfigFile(configFile);

    for (const auto& entry : configEntries) {
        logToFile("Exécution du test : " + entry.testID + " (" + entry.ctrlName + ")", logFileName);
        executeTest(entry.testID, entry.params, warningThreshold, logFileName);
    }
}

void generateConfigFile() {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> conn(driver->connect("tcp://localhost:3306", "root", "Mah010505!"));
        conn->setSchema("sys_monitor");

        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(
            "SELECT tc.control_id, tp.name, tc.description, tp.value, tp.value_2 "
            "FROM table_des_parametres tp "
            "JOIN table_des_controles tc ON tp.`control-id` = tc.control_id"
        ));

        std::ofstream configFile("config2.txt");

        if (configFile.is_open()) {
            while (res->next()) {
                int control_id = res->getInt("control_id");
                std::string name = res->getString("name");
                std::string description = res->getString("description");
                std::string value = res->getString("value");
                std::string value_2 = res->getString("value_2");

                std::cout << "Écriture : " << control_id << ";" << name << ";" << description << ";" << value << ";" << value_2 << std::endl;
                configFile << control_id << ";" << name << ";" << description << ";" << value << ";" << value_2 << std::endl;
            }

            configFile.close();
            std::cout << "Le fichier config2.txt a été généré avec succès." << std::endl;
        }
        else {
            std::cerr << "Impossible d'ouvrir le fichier config2.txt." << std::endl;
        }

    }
    catch (sql::SQLException& e) {
        std::cerr << "Erreur lors de la génération du fichier config2.txt : " << e.what() << std::endl;
    }
}

class MyMemoryResource : public std::pmr::memory_resource {
protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        return ::operator new(bytes); // Utilisez l'opérateur new standard
    }

    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
        ::operator delete(p); // Utilisez l'opérateur delete standard
    }

    bool do_is_equal(const memory_resource& other) const noexcept override {
        return this == &other; // Vérifie si les ressources sont identiques
    }
};

class DropdownExampleApp : public Wt::WApplication {
public:
    DropdownExampleApp(const Wt::WEnvironment& env)
        : Wt::WApplication(env) {
        // Définir le titre de la page web
        setTitle(Wt::WString::fromUTF8(u8"Système monitor"));

        // Créer un conteneur pour les widgets
        Wt::WContainerWidget* container = root();

        // Ajouter un texte pour afficher l'état de la sauvegarde
        statusText_ = container->addWidget(std::make_unique<Wt::WText>());

        // Ajouter un bouton pour ouvrir le fichier HTML
        //Wt::WPushButton* openHtmlButton = container->addWidget(std::make_unique<Wt::WPushButton>(
        //    Wt::WString::fromUTF8(u8"Télécharger HTML")));
        //openHtmlButton->clicked().connect([this]() {
        //    // Créer un lien vers le fichier HTML
        //    std::string htmlPageLink = "HTMLPage.htm"; // Chemin vers la page HTML
        //    redirect(htmlPageLink);
        //    });

        // Ajouter un bouton "New Config"
        Wt::WPushButton* newConfigButton = container->addWidget(std::make_unique<Wt::WPushButton>("New Config"));
        newConfigButton->clicked().connect([this, container]() {
            creerNouvelleConfiguration(container);
            });

        // Ajouter un bouton "Save Config" - garder cette instance uniquement
        Wt::WPushButton* saveConfigButton = container->addWidget(std::make_unique<Wt::WPushButton>("Save Config"));
        saveConfigButton->clicked().connect([this, container]() {
            sauvegarderConfiguration(container); // Appel correct de la fonction
            });

        // Ajouter un saut de ligne
        container->addWidget(std::make_unique<Wt::WBreak>());
    }

private:
    std::vector<std::pair<Wt::WComboBox*, Wt::WComboBox*>> configurations_;
    Wt::WText* statusText_; // Pour afficher le statut

    // Fonction pour sauvegarder la configuration
    void sauvegarderConfiguration(Wt::WContainerWidget* container) {
        std::stringstream fileStream;

        // Parcourez les configurations et récupérez les textes
        for (const auto& configPair : configurations_) {
            Wt::WComboBox* testComboBox = configPair.first;
            Wt::WComboBox* paramComboBox = configPair.second;

            std::string test = testComboBox->currentText().toUTF8();
            std::string param = paramComboBox->currentText().toUTF8();

            // Stockez le texte dans le flux
            fileStream << "Test : " << test << ", Paramètre : " << param << "\n";
        }

        std::string fileContent = fileStream.str();
        std::string fileName = "configurations.txt";

        // Écrire le contenu dans un fichier temporaire
        std::ofstream outFile(fileName, std::ios::out | std::ios::binary);
        if (outFile) {
            outFile << "\xEF\xBB\xBF"; // Ajout du BOM UTF-8
            outFile << fileContent;
            outFile.close();
        }
        else {
            statusText_->setText(Wt::WString::fromUTF8("Erreur lors de la création du fichier de configuration."));
            return;
        }

        // Utiliser Wt::WFileResource pour offrir le fichier en téléchargement
        auto fileResource = std::make_shared<Wt::WFileResource>("text/plain", fileName);
        fileResource->suggestFileName("configuration.txt");

        // Créer un lien de téléchargement et l'ajouter au conteneur
        Wt::WAnchor* downloadLink = container->addWidget(
            std::make_unique<Wt::WAnchor>(Wt::WLink(fileResource), "Télécharger la configuration"));

        // Mettre à jour le statut
        statusText_->setText(Wt::WString::fromUTF8("Configuration prête pour le téléchargement."));
    }

    void sauvegarderBoutonConfiguration(Wt::WContainerWidget* container) {
        std::stringstream fileStream;

        // Parcourez les configurations et récupérez les textes
        for (const auto& configPair : configurations_) {
            Wt::WComboBox* testComboBox = configPair.first;
            Wt::WComboBox* paramComboBox = configPair.second;

            std::string test = testComboBox->currentText().toUTF8();
            std::string param = paramComboBox->currentText().toUTF8();

            // Stockez le texte dans le flux
            fileStream << "Test : " << test << ", Paramètre : " << param << "\n";
        }

        std::string fileContent = fileStream.str();
        std::string fileName = "configurations.txt";

        // Créez le fichier avec l'encodage UTF-8
        std::ofstream outFile(fileName, std::ios::out | std::ios::binary); // Ouverture en mode binaire
        if (outFile) {
            // Ajoutez le BOM UTF-8 pour indiquer clairement l'encodage
            outFile << "\xEF\xBB\xBF";
            outFile << fileContent;
            outFile.close();
        }
        else {
            statusText_->setText(Wt::WString::fromUTF8("Erreur lors de la création du fichier."));
            return;
        }

        // Créez un lien pour télécharger le fichier
        Wt::WAnchor* downloadLink = container->addWidget(std::make_unique<Wt::WAnchor>(Wt::WLink(fileName), "Télécharger la configuration"));
        statusText_->setText(Wt::WString::fromUTF8("Configuration prête pour le téléchargement."));
    }

    // Fonction pour créer une nouvelle configuration
    void creerNouvelleConfiguration(Wt::WContainerWidget* container) {
        // Ajouter des widgets pour la nouvelle configuration
        Wt::WContainerWidget* configContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());

        // Ajouter un texte pour indiquer la sélection des tests
        configContainer->addWidget(std::make_unique<Wt::WText>("Sélectionnez un test dans la liste : ")); // Le "é" sera maintenant correct

        // Créer une première liste déroulante pour les tests
        Wt::WComboBox* testComboBox = configContainer->addWidget(std::make_unique<Wt::WComboBox>());
        testComboBox->addItem("Disk checker");
        testComboBox->addItem("Network checker");
        testComboBox->addItem("Removable drives checker");
        testComboBox->addItem("Service checker");

        // Ajouter un texte pour afficher la sélection du test
        Wt::WText* selectedTestText = configContainer->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Ajouter un deuxième bouton déroulant (qui sera caché au départ) pour les paramètres du test
        Wt::WComboBox* paramComboBox = configContainer->addWidget(std::make_unique<Wt::WComboBox>());
        paramComboBox->setHidden(true); // Cache le deuxième bouton déroulant au départ

        // Ajouter un texte pour afficher la sélection du paramètre
        Wt::WText* selectedParamText = configContainer->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Connecter l'événement 'changed' du bouton déroulant des tests
        testComboBox->changed().connect([=]() mutable {
            std::string selectedTest = testComboBox->currentText().toUTF8();
            selectedTestText->setText("Test sélectionné : " + selectedTest);

            // Réinitialiser les options du paramètre selon le test sélectionné
            paramComboBox->clear();
            if (selectedTest == "Disk checker") {
                paramComboBox->addItem("C:\\");
                paramComboBox->addItem("D:\\");
                paramComboBox->addItem("E:\\");
            }
            else if (selectedTest == "Network checker") {
                paramComboBox->addItem("Paramètre 2A");
                paramComboBox->addItem("Paramètre 2B");
                paramComboBox->addItem("Paramètre 2C");
            }
            else if (selectedTest == "Removable drives checker") {
                paramComboBox->addItem("Paramètre 3A");
                paramComboBox->addItem("Paramètre 3B");
                paramComboBox->addItem("Paramètre 3C");
            }
            else if (selectedTest == "Service checker") {
                paramComboBox->addItem("Paramètre 4A");
                paramComboBox->addItem("Paramètre 4B");
                paramComboBox->addItem("Paramètre 4C");
            }

            paramComboBox->setHidden(false); // Rendre visible la liste déroulante des paramètres
            });

        // Enregistrer les combobox dans la liste des configurations pour la sauvegarde
        configurations_.emplace_back(testComboBox, paramComboBox);

        // Ajouter un bouton "Add Test" pour ajouter plus de tests
        Wt::WPushButton* addTestButton = configContainer->addWidget(std::make_unique<Wt::WPushButton>("Ajouter un test"));
        addTestButton->clicked().connect([=]() {
            ajouterTestEtParametre(configContainer);
            });

        // Ajouter un saut de ligne après la configuration
        configContainer->addWidget(std::make_unique<Wt::WBreak>());
    }

    // Fonction pour ajouter un autre jeu de listes déroulantes (test et paramètres)
    void ajouterTestEtParametre(Wt::WContainerWidget* container) {
        // Ajouter un nouveau conteneur pour le test et les paramètres
        Wt::WContainerWidget* newTestContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());

        // Ajouter un texte pour sélectionner un autre test
        newTestContainer->addWidget(std::make_unique<Wt::WText>("Sélectionnez un autre test : "));

        // Créer une liste déroulante pour les tests
        Wt::WComboBox* newTestComboBox = newTestContainer->addWidget(std::make_unique<Wt::WComboBox>());
        newTestComboBox->addItem("Disk checker");
        newTestComboBox->addItem("Network checker");
        newTestComboBox->addItem("Removable drives checker");
        newTestComboBox->addItem("Service checker");

        // Ajouter un texte pour afficher la sélection du nouveau test
        Wt::WText* selectedNewTestText = newTestContainer->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Ajouter un deuxième bouton déroulant pour les paramètres du nouveau test
        Wt::WComboBox* newParamComboBox = newTestContainer->addWidget(std::make_unique<Wt::WComboBox>());
        newParamComboBox->setHidden(true); // Cache le deuxième bouton déroulant au départ

        // Ajouter un texte pour afficher la sélection du nouveau paramètre
        Wt::WText* selectedNewParamText = newTestContainer->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Connecter l'événement 'changed' de la nouvelle liste déroulante des tests
        newTestComboBox->changed().connect([=]() mutable {
            std::string selectedTest = newTestComboBox->currentText().toUTF8();
            selectedNewTestText->setText("Test sélectionné : " + selectedTest);

            // Réinitialiser les options du paramètre selon le nouveau test sélectionné
            newParamComboBox->clear();
            if (selectedTest == "Disk checker") {
                newParamComboBox->addItem("C:\\");
                newParamComboBox->addItem("D:\\");
                newParamComboBox->addItem("E:\\");
            }
            else if (selectedTest == "Network checker") {
                newParamComboBox->addItem("Paramètre 2A");
                newParamComboBox->addItem("Paramètre 2B");
                newParamComboBox->addItem("Paramètre 2C");
            }
            else if (selectedTest == "Removable drives checker") {
                newParamComboBox->addItem("Paramètre 3A");
                newParamComboBox->addItem("Paramètre 3B");
                newParamComboBox->addItem("Paramètre 3C");
            }
            else if (selectedTest == "Service checker") {
                newParamComboBox->addItem("Paramètre 4A");
                newParamComboBox->addItem("Paramètre 4B");
                newParamComboBox->addItem("Paramètre 4C");
            }

            newParamComboBox->setHidden(false); // Rendre visible la liste déroulante des paramètres
            });

        // Enregistrer les nouvelles combobox dans la liste des configurations
        configurations_.emplace_back(newTestComboBox, newParamComboBox);
    }
};

class Config {
public:
    Wt::Dbo::dbo_traits<int>::IdType config_id;
    std::string name;
    std::string desc;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, config_id, "config_id");
        Wt::Dbo::field(a, name, "name");
        Wt::Dbo::field(a, desc, "desc");
    }
};

class Control {
public:
    Wt::Dbo::dbo_traits<int>::IdType control_id;
    std::string name;
    std::string description;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, control_id, "control_id");
        Wt::Dbo::field(a, name, "name");
        Wt::Dbo::field(a, description, "description");
    }
};

class ConfigCtrl {
public:
    Wt::Dbo::dbo_traits<int>::IdType configid;
    Wt::Dbo::dbo_traits<int>::IdType ctrlid;

    Wt::Dbo::ptr<Config> config;   // Relation avec Config
    Wt::Dbo::ptr<Control> control; // Relation avec Control

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, configid, "configid");
        Wt::Dbo::field(a, ctrlid, "ctrlid");
        Wt::Dbo::belongsTo(a, config, "configid");
        Wt::Dbo::belongsTo(a, control, "ctrlid");
    }
};

class Parametre {
public:
    Wt::Dbo::dbo_traits<int>::IdType param_id;
    std::string name;
    std::string value;
    std::string value_2;
    Wt::Dbo::ptr<Control> control;  // Foreign key to `Control`

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, param_id, "param_id");
        Wt::Dbo::field(a, name, "name");
        Wt::Dbo::field(a, value, "value");
        Wt::Dbo::field(a, value_2, "value_2");
        Wt::Dbo::belongsTo(a, control, "control_id");  // Association avec Control
    }
};

void ajouterNouvelleConfig(Wt::WContainerWidget* container, Wt::Dbo::Session& session) {
    auto nameEdit = container->addWidget(std::make_unique<Wt::WLineEdit>());
    nameEdit->setPlaceholderText("Nom de la configuration");

    auto descEdit = container->addWidget(std::make_unique<Wt::WLineEdit>());
    descEdit->setPlaceholderText("Description de la configuration");

    auto submitButton = container->addWidget(std::make_unique<Wt::WPushButton>("Ajouter Configuration"));
    auto message = container->addWidget(std::make_unique<Wt::WText>());

    submitButton->clicked().connect([=, &session] {
        Wt::Dbo::Transaction transaction(session);

        // Créer un nouvel objet Config avec les valeurs entrées
        auto nouvelleConfig = session.add(std::make_unique<Config>());
        nouvelleConfig.modify()->name = nameEdit->text().toUTF8();
        nouvelleConfig.modify()->desc = descEdit->text().toUTF8();

        // Committer la transaction pour sauvegarder dans la base de données
        transaction.commit();

        message->setText("Nouvelle configuration ajoutée avec succès !");
        });
}

void ajouterNouveauParametre(Wt::WContainerWidget* container, Wt::Dbo::Session& session) {
    auto nameEdit = container->addWidget(std::make_unique<Wt::WLineEdit>());
    nameEdit->setPlaceholderText("Nom du paramètre");

    auto valueEdit = container->addWidget(std::make_unique<Wt::WLineEdit>());
    valueEdit->setPlaceholderText("Valeur du paramètre");

    auto controlIdEdit = container->addWidget(std::make_unique<Wt::WLineEdit>());
    controlIdEdit->setPlaceholderText("ID du contrôle associé");

    auto submitButton = container->addWidget(std::make_unique<Wt::WPushButton>("Ajouter Paramètre"));
    auto message = container->addWidget(std::make_unique<Wt::WText>());

    submitButton->clicked().connect([=, &session] {
        Wt::Dbo::Transaction transaction(session);

        int controlId = std::stoi(controlIdEdit->text().toUTF8());
        Wt::Dbo::ptr<Control> control = session.find<Control>().where("control_id = ?").bind(controlId);

        if (control) {
            auto nouveauParam = session.add(std::make_unique<Parametre>());
            nouveauParam.modify()->name = nameEdit->text().toUTF8();
            nouveauParam.modify()->value = valueEdit->text().toUTF8();
            nouveauParam.modify()->control = control;

            transaction.commit();
            message->setText("Nouveau paramètre ajouté avec succès !");
        }
        else {
            message->setText("Erreur: Contrôle introuvable.");
        }
        });
}

// Point d'entrée de l'application Wt
Wt::WApplication* createApplication(const Wt::WEnvironment& env) {
    auto app = std::make_unique<DropdownExampleApp>(env);

    auto container = app->root();  // Utilisation du conteneur racine

    // Créer une session Dbo pour gérer les transactions avec la base de données
    Wt::Dbo::backend::MySQL connection("sys_monitor", "root", "Mah010505!", "localhost");
    Wt::Dbo::Session session;
    session.setConnection(std::make_unique<Wt::Dbo::backend::MySQL>(connection));

    // Ajouter des champs pour la configuration
    ajouterNouvelleConfig(container, session);

    // Ajouter des champs pour les paramètres
    ajouterNouveauParametre(container, session);

    return app.release();
}

int main(int argc, char** argv) {
    return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
        return std::make_unique<DropdownExampleApp>(env);
        });
}

/*int main() {
    MyMemoryResource myResource;

    // Utilisation de myResource pour allouer de la mémoire
    std::pmr::polymorphic_allocator<int> alloc(&myResource);
    int* arr = alloc.allocate(10); // Allouer un tableau de 10 entiers

    // Utilisation d'arr ...

    alloc.deallocate(arr, 10); // Libérer la mémoire
    return 0;
}*/

//int main() {
//    int warningThreshold = 80;
//    int alertThreshold = 95;
//    std::string configFile = "config2.txt";
//    std::string logFileName = "system_monitor_log.txt";
//
//    logMachineInfo("system_monitor_log.txt");
//    executeTestsFromConfig(configFile, warningThreshold, logFileName);
//
//    return 0;
//}

//int main() {
//    generateConfigFile();
//    return 0;
//}
