#include "header.h"
using namespace std;

void network_checker_wrapper(const std::string& adapterName, const std::string& logFileName) {
    network_checker(adapterName, logFileName);
}

void list_removable_drives_wrapper(const std::string& params, const std::string& logFileName) {
    list_removable_drives(logFileName);
}

void executeTest(const std::string& testID, const std::string& params, int warningThreshold, const std::string& logFileName) {
    // Map pour les fonctions n�cessitant plusieurs param�tres (wchar_t*, int, string)
    std::map<std::string, std::function<void(const wchar_t*, int, const std::string&)>> testMapWithMultipleParams = {
        { "1", disk_free_space }  // Ne prend d�sormais qu'un seul seuil (warningThreshold)
    };

    // Map pour les fonctions n�cessitant un seul param�tre (params + logFileName)
    std::map<std::string, std::function<void(const std::string&, const std::string&)>> testMapWithParams = {
        { "2", network_checker_wrapper },
        { "3", list_removable_drives_wrapper }
    };

    // Map pour les fonctions sans param�tre (seulement logFileName)
    std::map<std::string, std::function<void(const std::string&)>> testMapNoParams = {
        { "4", ListRunningServices }
    };

    // Appel des fonctions avec plusieurs param�tres (comme pour disk_free_space)
    auto itWithMultipleParams = testMapWithMultipleParams.find(testID);
    if (itWithMultipleParams != testMapWithMultipleParams.end()) {
        std::stringstream ss(params);
        std::string driveLetter;
        std::getline(ss, driveLetter, ',');

        // Conversion de std::string en std::wstring pour Windows API
        std::wstring wDriveLetter(driveLetter.begin(), driveLetter.end());

        // Suppression du param�tre d'alerte, seule la valeur du seuil d'avertissement est utilis�e
        itWithMultipleParams->second(wDriveLetter.c_str(), warningThreshold, logFileName);
        return;
    }

    // Appel des fonctions avec un seul param�tre (params + logFileName)
    auto itWithParams = testMapWithParams.find(testID);
    if (itWithParams != testMapWithParams.end()) {
        itWithParams->second(params, logFileName);
        return;
    }

    // Appel des fonctions sans param�tre (juste logFileName)
    auto itNoParams = testMapNoParams.find(testID);
    if (itNoParams != testMapNoParams.end()) {
        itNoParams->second(logFileName);
        return;
    }

    // Si l'ID du test n'est pas trouv�
    logToFile("ID du test non trouv� : " + testID, logFileName);
}

void executeTestsFromConfig(const std::string& configFile, int warningThreshold, const std::string& logFileName) {
    std::vector<ConfigEntry> configEntries = readConfigFile(configFile);

    for (const auto& entry : configEntries) {
        logToFile("Ex�cution du test : " + entry.testID + " (" + entry.ctrlName + ")", logFileName);
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

                std::cout << "�criture : " << control_id << ";" << name << ";" << description << ";" << value << ";" << value_2 << std::endl;
                configFile << control_id << ";" << name << ";" << description << ";" << value << ";" << value_2 << std::endl;
            }

            configFile.close();
            std::cout << "Le fichier config2.txt a �t� g�n�r� avec succ�s." << std::endl;
        }
        else {
            std::cerr << "Impossible d'ouvrir le fichier config2.txt." << std::endl;
        }

    }
    catch (sql::SQLException& e) {
        std::cerr << "Erreur lors de la g�n�ration du fichier config2.txt : " << e.what() << std::endl;
    }
}


class DropdownExampleApp : public Wt::WApplication {
public:
    DropdownExampleApp(const Wt::WEnvironment& env)
        : Wt::WApplication(env) {

        // D�finir le titre de la page web
        setTitle("Systeme monitor");

        // Cr�er un conteneur pour les widgets
        Wt::WContainerWidget* container = root();

        // Ajouter un texte descriptif pour le premier bouton d�roulant
        container->addWidget(std::make_unique<Wt::WText>("S�lectionnez une option dans la premi�re liste : "));

        // Cr�er un premier bouton d�roulant (comboBox1)
        Wt::WComboBox* comboBox1 = container->addWidget(std::make_unique<Wt::WComboBox>());

        // Ajouter des options au premier bouton d�roulant
        comboBox1->addItem("Option 1");
        comboBox1->addItem("Option 2");
        comboBox1->addItem("Option 3");

        // Ajouter un texte pour afficher la s�lection du premier bouton
        Wt::WText* selectedText1 = container->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Cr�er un deuxi�me bouton d�roulant (qui sera cach� au d�part) pour comboBox1
        Wt::WComboBox* dynamicComboBox1 = container->addWidget(std::make_unique<Wt::WComboBox>());
        dynamicComboBox1->setHidden(true); // Cache le deuxi�me bouton d�roulant au d�part

        // Ajouter un texte pour afficher la s�lection du deuxi�me bouton
        Wt::WText* selectedText2 = container->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Connecter l'�v�nement 'changed' du premier bouton � une fonction
        comboBox1->changed().connect([this, comboBox1, dynamicComboBox1, selectedText1] {
            std::string selectedOption = comboBox1->currentText().toUTF8();
            selectedText1->setText("Premi�re liste s�lectionn�e : " + selectedOption);

            // R�initialiser les options du deuxi�me bouton d�roulant en fonction de la s�lection
            dynamicComboBox1->clear(); // Vide les options pr�c�dentes
            if (selectedOption == "Option 1") {
                dynamicComboBox1->addItem("Param�tre 1B");
                dynamicComboBox1->addItem("Param�tre 1A");
                dynamicComboBox1->addItem("Param�tre 1C");
            }
            else if (selectedOption == "Option 2") {
                dynamicComboBox1->addItem("Param�tre 2A");
                dynamicComboBox1->addItem("Param�tre 2B");
                dynamicComboBox1->addItem("Param�tre 2C");
            }
            else if (selectedOption == "Option 3") {
                dynamicComboBox1->addItem("Param�tre 3A");
                dynamicComboBox1->addItem("Param�tre 3B");
                dynamicComboBox1->addItem("Param�tre 3C");
            }

            // Rendre visible le deuxi�me bouton d�roulant apr�s la s�lection
            dynamicComboBox1->setHidden(false);
            });

        // Connecter l'�v�nement 'changed' du deuxi�me bouton d�roulant � une fonction
        dynamicComboBox1->changed().connect([this, dynamicComboBox1, selectedText2] {
            selectedText2->setText("Deuxi�me liste s�lectionn�e : " + dynamicComboBox1->currentText());
            });

        // Ajouter un saut de ligne avant le deuxi�me ensemble de widgets
        container->addWidget(std::make_unique<Wt::WBreak>());

        // Ajouter un texte descriptif pour le deuxi�me ensemble de boutons d�roulants
        container->addWidget(std::make_unique<Wt::WText>("S�lectionnez une option dans la deuxi�me liste : "));

        // Cr�er un deuxi�me bouton d�roulant (comboBox2)
        Wt::WComboBox* comboBox2 = container->addWidget(std::make_unique<Wt::WComboBox>());
        comboBox2->addItem("a");
        comboBox2->addItem("b");
        comboBox2->addItem("c");
        comboBox2->addItem("d");

        // Ajouter un texte pour afficher la s�lection du deuxi�me bouton
        Wt::WText* selectedText3 = container->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Cr�er un deuxi�me bouton d�roulant (qui sera cach� au d�part) pour comboBox2
        Wt::WComboBox* dynamicComboBox2 = container->addWidget(std::make_unique<Wt::WComboBox>());
        dynamicComboBox2->setHidden(true); // Cache le deuxi�me bouton d�roulant au d�part

        // Ajouter un texte pour afficher la s�lection du deuxi�me bouton
        Wt::WText* selectedText4 = container->addWidget(std::make_unique<Wt::WText>("<br/>"));

        // Connecter l'�v�nement 'changed' du comboBox2 � une fonction
        comboBox2->changed().connect([this, comboBox2, dynamicComboBox2, selectedText3] {
            std::string selectedOption = comboBox2->currentText().toUTF8();
            selectedText3->setText("Deuxi�me liste s�lectionn�e : " + selectedOption);

            // R�initialiser les options du deuxi�me bouton d�roulant en fonction de la s�lection dans comboBox2
            dynamicComboBox2->clear(); // Vide les options pr�c�dentes
            if (selectedOption == "a") {
                dynamicComboBox2->addItem("Param�tre a1");
                dynamicComboBox2->addItem("Param�tre a2");
            }
            else if (selectedOption == "b") {
                dynamicComboBox2->addItem("Param�tre b1");
                dynamicComboBox2->addItem("Param�tre b2");
            }
            else if (selectedOption == "c") {
                dynamicComboBox2->addItem("Param�tre c1");
                dynamicComboBox2->addItem("Param�tre c2");
            }
            else if (selectedOption == "d") {
                dynamicComboBox2->addItem("Param�tre d1");
                dynamicComboBox2->addItem("Param�tre d2");
            }

            // Rendre visible le deuxi�me bouton d�roulant apr�s la s�lection
            dynamicComboBox2->setHidden(false);
            });

        // Connecter l'�v�nement 'changed' du dynamicComboBox2 � une fonction
        dynamicComboBox2->changed().connect([this, dynamicComboBox2, selectedText4] {
            selectedText4->setText("Deuxi�me liste d�roulante s�lectionn�e : " + dynamicComboBox2->currentText());
            });

        // Ajouter un gestionnaire de fichiers
        Wt::WFileUpload* fu = container->addNew<Wt::WFileUpload>();
        fu->setProgressBar(std::make_unique<Wt::WProgressBar>());
        fu->setMargin(10, Wt::Side::Right);

        // D�finir un bouton pour d�marrer le t�l�chargement
        Wt::WPushButton* uploadButton = container->addNew<Wt::WPushButton>("Send");
        uploadButton->setMargin(10, Wt::Side::Left | Wt::Side::Right);

        // Zone de texte pour afficher les r�sultats du t�l�chargement
        Wt::WText* out = container->addNew<Wt::WText>();

        // T�l�chargement lorsque le bouton est cliqu�
        uploadButton->clicked().connect([this, fu, uploadButton, out] {
            fu->upload();
            uploadButton->disable();
            out->setText("Uploading file...");
            });

        // T�l�chargement automatique lorsque l'utilisateur s�lectionne un fichier
        fu->changed().connect([this, fu, uploadButton, out] {
            fu->upload();
            uploadButton->disable();
            out->setText("File selection changed. Uploading file...");
            });

        // R�agir � un t�l�chargement r�ussi
        fu->uploaded().connect([this, out, uploadButton] {
            out->setText("File upload finished successfully.");
            uploadButton->enable(); // R�active le bouton apr�s la fin du t�l�chargement
            });

        // R�agir � un probl�me de t�l�chargement
        fu->fileTooLarge().connect([this, out, uploadButton] {
            out->setText("File is too large.");
            uploadButton->enable(); // R�active le bouton si le fichier est trop volumineux
            });

        // Texte qui affichera l'�tat de sauvegarde
        Wt::WText* statusText = container->addWidget(std::make_unique<Wt::WText>());

        // Cr�er un bouton split avec menu d�roulant pour "Save" et "Save As"
        Wt::WSplitButton* sb = container->addWidget(std::make_unique<Wt::WSplitButton>("Save"));
        sb->setMargin(10, Wt::Side::Left);

        // Menu d�roulant pour "Save As"
        auto popup = std::make_unique<Wt::WPopupMenu>();
        auto popup_ = popup.get();
        popup_->addItem("Save As ...");

        // Ajouter le menu d�roulant au bouton
        sb->dropDownButton()->setMenu(std::move(popup));

        // Connecter le bouton "Save" � l'enregistrement local
        sb->actionButton()->clicked().connect([this, comboBox1, comboBox2, statusText] {
            std::string choice1 = comboBox1->currentText().toUTF8();
            std::string choice2 = comboBox2->currentText().toUTF8();
            statusText->setText("Choix enregistr�s : " + choice1 + ", " + choice2);
            });

        // Connecter l'option "Save As" pour enregistrer les choix dans un fichier texte
        popup_->itemSelected().connect([this, comboBox1, comboBox2, dynamicComboBox1, dynamicComboBox2, statusText](Wt::WMenuItem* item) {
            if (item && item->text() == "Save As ...") {
                std::ofstream file("choix_utilisateur.txt");
                if (file.is_open()) {
                    std::string choice1 = comboBox1->currentText().toUTF8();
                    std::string choice2 = comboBox2->currentText().toUTF8();
                    std::string choice_param1 = dynamicComboBox1->currentText().toUTF8(); // R�cup�re le texte s�lectionn�
                    std::string choice_param2 = dynamicComboBox2->currentText().toUTF8(); // R�cup�re le texte s�lectionn�
                    file << "Choix 1 : " << choice1 << " ";
                    file << "param�tre : " << choice_param1 << "\n";  // �crit le texte s�lectionn�
                    file << "Choix 2 : " << choice2 << " ";
                    file << "param�tre : " << choice_param2 << "\n";  // �crit le texte s�lectionn�
                    file.close();
                    statusText->setText("Choix sauvegard�s dans 'choix_utilisateur.txt'.");
                }
                else {
                    statusText->setText("Erreur : Impossible de cr�er le fichier.");
                }
            }
            });
    }
};

// Point d'entr�e de l'application Wt
Wt::WApplication* createApplication(const Wt::WEnvironment& env) {
    return new DropdownExampleApp(env);
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
