#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <td/telegram/td_api.h>
#include <td/telegram/Client.h>
#include <nlohmann/json.hpp>
#include <exception>

using namespace td;
using namespace td_api;
using json = nlohmann::json;

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

struct list_of_chats
{
    int count;
    int64_t chat_id;
    std::string name;
    list_of_chats(int c, int64_t id, std::string n): count(c), chat_id(id), name(n){}
};

bool authorization (ClientManager &client, int32_t client_id, Config config_)
{
    auto parametrs = make_object<setTdlibParameters>();
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
        if (response.object && response.object -> get_id() == error::ID)
        {
            auto err = move_object_as<error>(response.object);
            std::cout << "Auth error" << " " << err -> code_ << ": " << err -> message_ << std::endl;
            if (err -> message_ == "PHONE_CODE_INVALID")
            {
                std::cout << "The code you entered is invalid. Try again." << std::endl;
                break;
            }
            if (err -> message_ == "PASSWORD_HASH-INVALID")
            {
                std::cout << "Wrong 2fa password. Try again" << std::endl;
                break;
            }
        }

        if (response.object)
        {
            if (response.object -> get_id() == updateAuthorizationState::ID)
            {
                auto updateAuthState = move_object_as<updateAuthorizationState>(response.object);
                switch (updateAuthState -> authorization_state_ -> get_id())
                {
                    case authorizationStateWaitPhoneNumber::ID:
                    {
                            while (true)
                            {
                                try
                                {
                                    std::string phone;
                                    std::cout << "Input phone number: ";
                                    getline(std::cin, phone);
                                    phone.erase(std::remove_if(phone.begin(), phone.end(), ::isspace), phone.end());
                                    if (phone.empty() ||  phone.length() < 8 || phone.length() > 15 || !std::all_of(phone.begin(), phone.end(), ::isdigit))
                                    {
                                        throw std::runtime_error("Phone number incorrect!\n");
                                    }
                                    auto settings = make_object<phoneNumberAuthenticationSettings>();
                                    settings -> allow_flash_call_ = false;
                                    settings -> allow_sms_retriever_api_= false;

                                    client.send(client_id, 2, make_object<setAuthenticationPhoneNumber>(phone, std::move(settings)));
                                    break;
                                } catch (const std::exception& e) { std::cout << e.what() << "Try again!" << std::endl; }
                            }
                            break;
                    }
                    case authorizationStateWaitCode::ID:
                        {
                            while (true){
                                try
                                {
                                    std::string code;
                                    std::cout << "Paste the code : ";
                                    std::cin >> code;
                                    code.erase(std::remove_if(code.begin(), code.end(), ::isspace), code.end());

                                    if (code.empty() || code.length() != 5 || !std::all_of(code.begin(), code.end(), ::isdigit))
                                    {
                                        throw std::runtime_error("The code is invalid. It should be only digits.");
                                    }
                                    client.send(client_id, 3, make_object<checkAuthenticationCode>(code));
                                    break;
                                } catch (const std::exception& e) { std::cout << e.what() << "Try again!" << std::endl; }
                            }
                            break;
                        }
                    case authorizationStateWaitPassword::ID:
                    {
                            while (true)
                            {
                                try
                                {
                                    std::string _2fa;
                                    std::cout << "\n2fa password: ";
                                    std::cin >> _2fa;

                                    if (_2fa.empty())
                                    {
                                        throw std::runtime_error("Password cannot be empty!");
                                    }

                                    client.send(client_id, 4, make_object<checkAuthenticationPassword>(_2fa));
                                    break;
                                } catch (const std::exception& e) { std::cout << e.what() << "Try again!" << std::endl; }
                            }
                            break;
                    }
                    case authorizationStateReady::ID:
                        {
                            std::cout << "\nAuthorization completed!\n" << std::endl;
                            client.send(client_id, 6, make_object<getChats>(nullptr, 100000));
                            authorized = true;
                            break;
                        }
                    case authorizationStateClosed::ID:
                        {
                            std::cout << "\nConnection closed. Restart the program." << std::endl;
                            return false;
                            break;
                        }
                    default:
                        {
                            std::cout << "Waiting next step..." << std::endl;
                            break;
                        }
                    }
                }
            }
        }
    return true;
}

std::vector<std::unique_ptr<list_of_chats>> get_chats(ClientManager &client, int32_t client_id)
{
    std::vector<std::unique_ptr<list_of_chats>> arr;
    int count = 0;

    while (true)
    {
        auto response = client.receive(5.0);
        if (!response.object) continue;
        if (response.object -> get_id() != chats::ID) continue;
        auto chats_id = move_object_as<chats>(response.object);
        if (!chats_id) continue;
        std::cout << "Chats: " << std::endl;
        for (auto id: chats_id -> chat_ids_)
            {
            client.send(client_id, 7, make_object<getChat>(id));
            auto query = client.receive(5.0);

            if (!query.object) continue;
            if (query.object -> get_id() != chat::ID) continue;

            auto chatinfo = move_object_as<chat>(query.object);
            if (!chatinfo) continue;

            if (chatinfo  && (chatinfo -> type_ -> get_id() == chatTypeBasicGroup::ID || chatinfo -> type_ -> get_id() == chatTypePrivate::ID || chatinfo -> type_ -> get_id() == chatTypeSupergroup::ID || chatinfo -> type_ -> get_id() == chatTypeSecret::ID))
                {
                ++count;
                std::cout << count << " " << id << " " << chatinfo -> title_ << std::endl;
                arr.push_back(std::make_unique<list_of_chats>(count, id, chatinfo -> title_));
                }
            }
        break;
    }
    return arr;
}

void delete_messages(ClientManager &client, int32_t client_id, std::vector<std::unique_ptr<list_of_chats>> &arr, int choice, int64_t my_user_id)
{
    int64_t chat_id = arr[choice - 1]->chat_id;
    int64_t from_msg_id = 0;
    int count = 0;

    bool can_delete_all = false;
    auto sender = make_object<messageSenderUser>(my_user_id);
    client.send(client_id, 10, make_object<getChatMember>(chat_id, std::move(sender)));
    auto member_response = client.receive(5.0);

    if (member_response.object && member_response.object->get_id() == chatMember::ID) {
        auto member = move_object_as<chatMember>(member_response.object);
        if (member->status_->get_id() == chatMemberStatusAdministrator::ID || member->status_->get_id() == chatMemberStatusCreator::ID)
            {
            can_delete_all = true;
            }
    }

    while (true)
    {
        std::vector<int64_t> messages_id;
        client.send(client_id, 8, make_object<getChatHistory>(chat_id, from_msg_id, 0, 100, false));
        auto response = client.receive(5.0);

        if (!response.object)
        {
            ++count;
            if (count > 5)
            {
                std::cout << "Timeout or no messages." << std::endl;
                break;
            }
            continue;
        }
        count = 0;

        if (response.object->get_id() == messages::ID)
        {
            auto msg_ids = move_object_as<messages>(response.object);
            if (msg_ids->messages_.empty())
            {
                std::cout << "Chat cleared!" << std::endl;
                break;
            }

            for (auto &msg : msg_ids->messages_)
            {
                if (can_delete_all) {
                    messages_id.push_back(msg->id_);
                } else if (msg->sender_id_->get_id() == messageSenderUser::ID) {
                    auto sender = static_cast<messageSenderUser*>(msg->sender_id_.get());
                    if (sender->user_id_ == my_user_id) {
                        messages_id.push_back(msg->id_);
                    }
                }
            }

            if (!messages_id.empty())
                client.send(client_id, 9, make_object<deleteMessages>(chat_id, std::move(messages_id), true));

            from_msg_id = msg_ids->messages_.back()->id_;
        }
        else
        {
            std::cout << "Received unexpected object id: " << response.object->get_id() << std::endl;
        }
    }
}


int main()
{
    ClientManager client;
    int32_t client_id = client.create_client_id();
    Config config_ = load_config();
    client.send(client_id, 12345, make_object<setLogVerbosityLevel>(0));

    if (!authorization(client, client_id, config_))
    {
        std::cout << "Authorization failed!" << std::endl;
        return 1;
    }

    int64_t my_user_id = 0;
    client.send(client_id, 200, make_object<getMe>());
    while (true)
    {
        auto response = client.receive(5.0);
        if (!response.object) continue;

        if (response.object->get_id() == user::ID)
        {
            auto me = move_object_as<user>(response.object);
            my_user_id = me->id_;
            break;
        }
    }

    auto chats = get_chats(client, client_id);
    if (chats.empty())
    {
        std::cout << "No chats found." << std::endl;
        return 1;
    }

    int choice;
    std::cout << "Select chat to clear: ";
    std::cin >> choice;
    delete_messages(client, client_id, chats, choice, my_user_id);

    return 0;
}
