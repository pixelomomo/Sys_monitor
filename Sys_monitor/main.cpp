#include "header.h"
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#pragma warning(disable : 4275)
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
        // Set the title of the web page
        setTitle(Wt::WString::fromUTF8(u8"Système monitor"));

        // Create a container for widgets
        Wt::WContainerWidget* container = root();

        // Add a text to display the save status
        statusText_ = container->addWidget(std::make_unique<Wt::WText>());

        // Add a button to open the HTML file
        Wt::WPushButton* openHtmlButton = container->addWidget(std::make_unique<Wt::WPushButton>(
            Wt::WString::fromUTF8(u8"Télécharger la config")));
        openHtmlButton->clicked().connect([this]() {
            // Create a link to the HTML page
            std::string htmlPageLink = "HTMLPage.htm"; // Path to your HTML page

            // Redirect to the HTML page
            Wt::WApplication::instance()->redirect(htmlPageLink);
            });

        // Add a "New Config" button
        Wt::WPushButton* newConfigButton = root()->addWidget(std::make_unique<Wt::WPushButton>("New Config"));
        newConfigButton->clicked().connect([this, container]() {
            creerNouvelleConfiguration(container);
            });

        // Add a "Save Config" button
        Wt::WPushButton* saveConfigButton = container->addWidget(std::make_unique<Wt::WPushButton>("Save Config"));
        saveConfigButton->clicked().connect([this, container]() {
            sauvegarderConfiguration(container); // Call the function correctly
            });

        // Add a line break
        container->addWidget(std::make_unique<Wt::WBreak>());
    }

private:
    std::vector<std::pair<Wt::WComboBox*, Wt::WComboBox*>> configurations_;
    Wt::WText* statusText_; // To display status

    // Function to save the configuration
    void sauvegarderConfiguration(Wt::WContainerWidget* container) {
        std::stringstream fileStream;

        for (const auto& configPair : configurations_) {
            Wt::WComboBox* testComboBox = configPair.first;
            Wt::WComboBox* paramComboBox = configPair.second;

            // Get the text in UTF-8
            std::string test = testComboBox->currentText().toUTF8();
            std::string param = paramComboBox->currentText().toUTF8();

            // Write into the stream
            fileStream << "Test : " << test << ", Paramètre : " << param << "\n";
        }

        // File content
        std::string fileContent = fileStream.str();
        std::string fileName = "configurations.txt";

        // Open the file in UTF-8
        std::ofstream outFile(fileName);
        if (outFile) {
            outFile << fileContent;  // Write into the file
            outFile.close();
        }
        else {
            statusText_->setText(Wt::WString::fromUTF8("Erreur lors de la création du fichier de configuration."));
            return;
        }

        // Create a download link
        Wt::WAnchor* downloadLink = container->addWidget(std::make_unique<Wt::WAnchor>(Wt::WLink(fileName), "Télécharger la configuration"));

        statusText_->setText(Wt::WString::fromUTF8("Configuration prête pour le téléchargement."));
    }

    // Function to create a new configuration
    void creerNouvelleConfiguration(Wt::WContainerWidget* container) {
        // Add widgets for the new configuration
        Wt::WContainerWidget* configContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());

        // Add text to indicate the test selection
        configContainer->addWidget(std::make_unique<Wt::WText>("Sélectionnez un test dans la liste : "));

        // Create the first dropdown for tests
        Wt::WComboBox* testComboBox = configContainer->addWidget(std::make_unique<Wt::WComboBox>());
        testComboBox->addItem("Disk checker");
        testComboBox->addItem("Network checker");
        testComboBox->addItem("Removable drives checker");
        testComboBox->addItem("Service checker");

        // Add text to display the selected test
        Wt::WText* selectedTestText = configContainer->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Add a second hidden dropdown for test parameters
        Wt::WComboBox* paramComboBox = configContainer->addWidget(std::make_unique<Wt::WComboBox>());
        paramComboBox->setHidden(true); // Hide the second dropdown initially

        // Add text to display the selected parameter
        Wt::WText* selectedParamText = configContainer->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Connect the 'changed' event of the test dropdown
        testComboBox->changed().connect([=]() mutable {
            std::string selectedTest = testComboBox->currentText().toUTF8();
            selectedTestText->setText("Test sélectionné : " + selectedTest);

            // Reset the parameter options based on the selected test
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

            paramComboBox->setHidden(false); // Make the parameter dropdown visible
            });

        // Store the comboboxes in the configurations vector for saving
        configurations_.emplace_back(testComboBox, paramComboBox);

        // Add a line break after the configuration
        configContainer->addWidget(std::make_unique<Wt::WBreak>());
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
