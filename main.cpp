#include <iostream>
#include <algorithm>
#include <limits>
#include <td/telegram/td_api.h>
#include <td/telegram/Client.h>
using namespace td;
using namespace td_api;

int main()
{
    td::ClientManager client;
    auto client_id = client.create_client_id();

    auto parametrs = make_object<td::td_api::setTdlibParameters>();
    parametrs -> database_directory_ = "./dataSession";
    parametrs -> use_message_database_ = true;
    parametrs -> use_secret_chats_ = true;
    parametrs -> system_language_code_ = "en";
    parametrs -> device_model_ = "PC";
    parametrs -> system_version_ = "TGCleaner";
    parametrs -> application_version_ = "1.0";
    std::cout << "Input api_id: ";
    int api_id;
    std::cin >> api_id;
    parametrs -> api_id_ = api_id;

    std::cout << "\nInput api_hash: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //нужна очистка буфера после cin
    getline(std::cin, parametrs -> api_hash_);

    client.send(client_id, 1, std::move(parametrs));

    while (true)
    {
        auto response = client.receive(5.0);

        if (!response.object) continue;
        if (response.object)
        {
            if (response.object -> get_id() == updateAuthorizationState::ID)
            {
                // получаем объект конкретного типа. теперь updateAuthState это умный указатель
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

                            client.send(client_id, 2, make_object<setAuthenticationPhoneNumber>(phoneNumber, nullptr));
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
                            client.send(client_id, 5, make_object<getMe>());
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
    return 0;
}