#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <td/telegram/td_api.h>
#include <td/telegram/Client.h>
#include <nlohmann/json.hpp>

using namespace td;
using namespace td_api;
using json = nlohmann::json;

/* env function */
struct Config
{
    int api_id;
    std::string api_hash;
    std::string key;
};

Config load_config(const std::string& filepath = "config.json")
{
    Config config;
    std::ifstream in_file(filepath);
    if (!in_file)
    {
        std::cout << "Password for database: " << std::endl;
        std::cin >> config.key;
        std::cout << "Input api_id: " << std::endl;
        std::cin >> config.api_id;
        std::cout << "Input api_hash: " << std::endl;
        std::cin >> config.api_hash;

        json j;
        j["database_key"] = config.key;
        j["api_id"] = config.api_id;
        j["api_hash"] = config.api_hash;
        std::ofstream output(filepath);
        output << j.dump(4);
        std::cout << "Config saved" << std::endl;
    } else
    {
        json j;
        in_file >> j;
        config.api_id = j["api_id"];
        config.api_hash = j["api_hash"];
        config.key = j["database_key"];
    }
    return config;
}

int main()
{
    td::ClientManager client;
    auto client_id = client.create_client_id();
    client.send(client_id, 12345, make_object<setLogVerbosityLevel>(0));
    auto parametrs = make_object<td::td_api::setTdlibParameters>();
    Config config_ = load_config();

    parametrs -> database_directory_ = "/home/ivanoaof/tgcleansing/dataSession";
    parametrs -> use_message_database_ = true;
    parametrs -> use_secret_chats_ = true;
    parametrs -> system_language_code_ = "en";
    parametrs -> device_model_ = "PC";
    parametrs -> system_version_ = "TGCleaner";
    parametrs -> application_version_ = "1.0";
    parametrs -> database_encryption_key_ = config_.key;
    parametrs -> api_id_ = config_.api_id;
    parametrs -> api_hash_ = config_.api_hash;

    client.send(client_id, 1, std::move(parametrs));
    bool authorized = false;
    while (!authorized)
    {
        auto response = client.receive(5.0);

        if (!response.object) continue;
        if (response.object)
        {
            if (response.object -> get_id() == updateAuthorizationState::ID)
            {
                /* получаем объект конкретного типа. теперь updateAuthState это умный указатель */
                auto updateAuthState = move_object_as<updateAuthorizationState>(response.object);
                switch (updateAuthState -> authorization_state_ -> get_id())
                {
                    case authorizationStateWaitPhoneNumber::ID:
                    {
                            std::string phoneNumber;
                            std::cout << "\nInput phone number: ";
                            std::getline(std::cin, phoneNumber);

                            auto settings = make_object<phoneNumberAuthenticationSettings>();
                            settings ->allow_flash_call_ = false;
                            settings -> allow_sms_retriever_api_= false;

                            client.send(client_id, 2, make_object<setAuthenticationPhoneNumber>(phoneNumber, std::move(settings)));
                            break;
                    }
                    case authorizationStateWaitCode::ID:
                    {
                            std::string code;
                            std::cout << "\nPaste the code: ";
                            std::cin >> code;
                            client.send(client_id, 3, make_object<checkAuthenticationCode>(code));
                            break;
                    }
                    case authorizationStateWaitPassword::ID:
                        {
                            std::string _2fa;
                            std::cout << "\n2fa password: ";
                            std::cin >> _2fa;
                            client.send(client_id, 4, make_object<checkAuthenticationPassword>(_2fa));
                            break;
                        }
                    case authorizationStateReady::ID:
                        {
                            std::cout << "\nAuthorization completed!" << std::endl;
                            client.send(client_id, 6, make_object<getChats>(nullptr, 100000));
                            authorized = true;
                            break;
                        }
                    case authorizationStateClosed::ID:
                        {
                            std::cout << "\n Connection closed. Restart the program." << std::endl;
                            return 1;
                        }
                    default:
                        {
                            std::cout << "Waiting next step." << std::endl;
                            break;
                        }
                    }
                }
            }
        }

    std::ofstream file("/home/ivanoaof/tgcleansing/ids.txt");
    while (true)
    {
        auto response = client.receive(5.0);
        if (!response.object) continue;
        if (response.object -> get_id() == chats::ID)
        {
            auto chats_id = move_object_as<chats>(response.object);
            if (!chats_id) continue;
            std::cout << "Chats id: " << std::endl;
            for (auto id: chats_id -> chat_ids_)
            {
                client.send(client_id, 7, make_object<getChat>(id));
                auto query = client.receive(5.0);
                auto chatinfo = move_object_as<chat>(query.object);
                if (chatinfo && chatinfo != nullptr && chatinfo -> type_ -> get_id() == chatTypeBasicGroup::ID || chatinfo -> type_ -> get_id() == chatTypePrivate::ID || chatinfo -> type_ -> get_id() == chatTypeSupergroup::ID || chatinfo -> type_ -> get_id() == chatTypeSecret::ID)
                {
                    std::cout << id << " " << chatinfo -> title_ << std::endl;
                    file << id << " " << chatinfo -> title_ << std::endl;
                }
            }
            break;
        }
    }
    return 0;
}