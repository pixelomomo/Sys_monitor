#include "header.h"
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


class DropdownExampleApp : public Wt::WApplication {
public:
    DropdownExampleApp(const Wt::WEnvironment& env)
        : Wt::WApplication(env) {

        // Définir le titre de la page web
        setTitle("Systeme monitor");

        // Créer un conteneur pour les widgets
        Wt::WContainerWidget* container = root();

        // Ajouter un texte descriptif pour le premier bouton déroulant
        container->addWidget(std::make_unique<Wt::WText>("Sélectionnez une option dans la première liste : "));

        // Créer un premier bouton déroulant (comboBox1)
        Wt::WComboBox* comboBox1 = container->addWidget(std::make_unique<Wt::WComboBox>());

        // Ajouter des options au premier bouton déroulant
        comboBox1->addItem("Disk checker");
        comboBox1->addItem("Network checker");
        comboBox1->addItem("Removable drives checker");
        comboBox1->addItem("Service checker");

        // Ajouter un texte pour afficher la sélection du premier bouton
        Wt::WText* selectedText1 = container->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Créer un deuxième bouton déroulant (qui sera caché au départ) pour comboBox1
        Wt::WComboBox* dynamicComboBox1 = container->addWidget(std::make_unique<Wt::WComboBox>());
        dynamicComboBox1->setHidden(true); // Cache le deuxième bouton déroulant au départ

        // Ajouter un texte pour afficher la sélection du deuxième bouton
        Wt::WText* selectedText2 = container->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Connecter l'événement 'changed' du premier bouton à une fonction
        comboBox1->changed().connect([this, comboBox1, dynamicComboBox1, selectedText1] {
            std::string selectedOption = comboBox1->currentText().toUTF8();
            selectedText1->setText("Première liste sélectionnée : " + selectedOption);

            // Réinitialiser les options du deuxième bouton déroulant en fonction de la sélection
            dynamicComboBox1->clear(); // Vide les options précédentes
            if (selectedOption == "Disk checker") {
                dynamicComboBox1->addItem("C:\\");
                dynamicComboBox1->addItem("D:\\");
                dynamicComboBox1->addItem("E:\\");
            }
            else if (selectedOption == "Network checker") {
                dynamicComboBox1->addItem("Paramètre 2A");
                dynamicComboBox1->addItem("Paramètre 2B");
                dynamicComboBox1->addItem("Paramètre 2C");
            }
            else if (selectedOption == "Removable drives checker") {
                dynamicComboBox1->addItem("Paramètre 3A");
                dynamicComboBox1->addItem("Paramètre 3B");
                dynamicComboBox1->addItem("Paramètre 3C");
            }
            else if (selectedOption == "Service checker") {
                dynamicComboBox1->addItem("Paramètre 4A");
                dynamicComboBox1->addItem("Paramètre 4B");
                dynamicComboBox1->addItem("Paramètre 4C");
            }

            // Rendre visible le deuxième bouton déroulant après la sélection
            dynamicComboBox1->setHidden(false);
            });

        // Connecter l'événement 'changed' du deuxième bouton déroulant à une fonction
        dynamicComboBox1->changed().connect([this, dynamicComboBox1, selectedText2] {
            selectedText2->setText("Deuxième liste sélectionnée : " + dynamicComboBox1->currentText());
            });

        // Ajouter un saut de ligne avant le deuxième ensemble de widgets
        container->addWidget(std::make_unique<Wt::WBreak>());

        // Ajouter un texte descriptif pour le deuxième ensemble de boutons déroulants
        container->addWidget(std::make_unique<Wt::WText>("Sélectionnez une option dans la deuxième liste : "));

        // Créer un deuxième bouton déroulant (comboBox2)
        Wt::WComboBox* comboBox2 = container->addWidget(std::make_unique<Wt::WComboBox>());
        comboBox2->addItem("a");
        comboBox2->addItem("b");
        comboBox2->addItem("c");
        comboBox2->addItem("d");

        // Ajouter un texte pour afficher la sélection du deuxième bouton
        Wt::WText* selectedText3 = container->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Créer un deuxième bouton déroulant (qui sera caché au départ) pour comboBox2
        Wt::WComboBox* dynamicComboBox2 = container->addWidget(std::make_unique<Wt::WComboBox>());
        dynamicComboBox2->setHidden(true); // Cache le deuxième bouton déroulant au départ

        // Ajouter un texte pour afficher la sélection du deuxième bouton
        Wt::WText* selectedText4 = container->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Connecter l'événement 'changed' du comboBox2 à une fonction
        comboBox2->changed().connect([this, comboBox2, dynamicComboBox2, selectedText3] {
            std::string selectedOption = comboBox2->currentText().toUTF8();
            selectedText3->setText("Deuxième liste sélectionnée : " + selectedOption);

            // Réinitialiser les options du deuxième bouton déroulant en fonction de la sélection dans comboBox2
            dynamicComboBox2->clear(); // Vide les options précédentes
            if (selectedOption == "a") {
                dynamicComboBox2->addItem("Paramètre a1");
                dynamicComboBox2->addItem("Paramètre a2");
            }
            else if (selectedOption == "b") {
                dynamicComboBox2->addItem("Paramètre b1");
                dynamicComboBox2->addItem("Paramètre b2");
            }
            else if (selectedOption == "c") {
                dynamicComboBox2->addItem("Paramètre c1");
                dynamicComboBox2->addItem("Paramètre c2");
            }
            else if (selectedOption == "d") {
                dynamicComboBox2->addItem("Paramètre d1");
                dynamicComboBox2->addItem("Paramètre d2");
            }

            // Rendre visible le deuxième bouton déroulant après la sélection
            dynamicComboBox2->setHidden(false);
            });

        // Connecter l'événement 'changed' du dynamicComboBox2 à une fonction
        dynamicComboBox2->changed().connect([this, dynamicComboBox2, selectedText4] {
            selectedText4->setText("Deuxième liste déroulante sélectionnée : " + dynamicComboBox2->currentText());
            });

        // Ajouter un gestionnaire de fichiers
        Wt::WFileUpload* fu = container->addNew<Wt::WFileUpload>();
        fu->setProgressBar(std::make_unique<Wt::WProgressBar>());
        fu->setMargin(10, Wt::Side::Right);

        // Définir un bouton pour démarrer le téléchargement
        Wt::WPushButton* uploadButton = container->addNew<Wt::WPushButton>("Send");
        uploadButton->setMargin(10, Wt::Side::Left | Wt::Side::Right);

        // Zone de texte pour afficher les résultats du téléchargement
        Wt::WText* out = container->addNew<Wt::WText>();

        // Téléchargement lorsque le bouton est cliqué
        uploadButton->clicked().connect([this, fu, uploadButton, out] {
            fu->upload();
            uploadButton->disable();
            out->setText("Uploading file...");
            });

        // Téléchargement automatique lorsque l'utilisateur sélectionne un fichier
        fu->changed().connect([this, fu, uploadButton, out] {
            fu->upload();
            uploadButton->disable();
            out->setText("File selection changed. Uploading file...");
            });

        // Réagir à un téléchargement réussi
        fu->uploaded().connect([this, out, uploadButton] {
            out->setText("File upload finished successfully.");
            uploadButton->enable(); // Réactive le bouton après la fin du téléchargement
            });

        // Réagir à un problème de téléchargement
        fu->fileTooLarge().connect([this, out, uploadButton] {
            out->setText("File is too large.");
            uploadButton->enable(); // Réactive le bouton si le fichier est trop volumineux
            });

        // Texte qui affichera l'état de sauvegarde
        Wt::WText* statusText = container->addWidget(std::make_unique<Wt::WText>());

        // Créer un bouton split avec menu déroulant pour "Save" et "Save As"
        Wt::WSplitButton* sb = container->addWidget(std::make_unique<Wt::WSplitButton>("Save"));
        sb->setMargin(10, Wt::Side::Left);

        // Menu déroulant pour "Save As"
        auto popup = std::make_unique<Wt::WPopupMenu>();
        auto popup_ = popup.get();
        popup_->addItem("Save As ...");

        // Ajouter le menu déroulant au bouton
        sb->dropDownButton()->setMenu(std::move(popup));

        // Connecter le bouton "Save" à l'enregistrement local
        sb->actionButton()->clicked().connect([this, comboBox1, comboBox2, statusText] {
            std::string choice1 = comboBox1->currentText().toUTF8();
            std::string choice2 = comboBox2->currentText().toUTF8();
            statusText->setText("Choix enregistrés : " + choice1 + ", " + choice2);
            });

        // Connecter l'option "Save As" pour enregistrer les choix dans un fichier texte
        popup_->itemSelected().connect([this, comboBox1, comboBox2, dynamicComboBox1, dynamicComboBox2, statusText](Wt::WMenuItem* item) {
            if (item && item->text() == "Save As ...") {
                std::ofstream file("choix_utilisateur.txt");
                if (file.is_open()) {
                    std::string choice1 = comboBox1->currentText().toUTF8();
                    std::string choice2 = comboBox2->currentText().toUTF8();
                    std::string choice_param1 = dynamicComboBox1->currentText().toUTF8(); // Récupère le texte sélectionné
                    std::string choice_param2 = dynamicComboBox2->currentText().toUTF8(); // Récupère le texte sélectionné
                    file << "Choix 1 : " << choice1 << " ";
                    file << "paramètre : " << choice_param1 << "\n";  // Écrit le texte sélectionné
                    file << "Choix 2 : " << choice2 << " ";
                    file << "paramètre : " << choice_param2 << "\n";  // Écrit le texte sélectionné
                    file.close();
                    statusText->setText("Choix sauvegardés dans 'choix_utilisateur.txt'.");
                }
                else {
                    statusText->setText("Erreur : Impossible de créer le fichier.");
                }
            }
            });
        /*container->addWidget(std::make_unique<Wt::WText>("Tentative de connexion à MySQL...<br/>"));

        try {
            auto mysqlBackend = std::make_unique<Wt::Dbo::backend::MySQL>("sys_monitor", "root", "Mah010505!", "localhost", 3306);
            Wt::Dbo::Session session;
            session.setConnection(std::move(mysqlBackend));
            session.createTables(); // Create tables based on models
            container->addWidget(std::make_unique<Wt::WText>("Connexion réussie à la base de données MySQL.<br/>"));
        }
        catch (const Wt::Dbo::Exception& e) {
            container->addWidget(std::make_unique<Wt::WText>("Erreur de connexion : " + std::string(e.what()) + "<br/>"));
        }
        catch (const std::exception& e) {
            container->addWidget(std::make_unique<Wt::WText>("Erreur inattendue : " + std::string(e.what()) + "<br/>"));
        }
    }*/
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
