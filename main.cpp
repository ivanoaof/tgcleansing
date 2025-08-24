#include <iostream>
#include <algorithm>
#include <td/telegram/td_api.h>
#include <td/telegram/Client.h>

using td::td_api::make_object;
using td::td_api::getMe;
using td::td_api::updateAuthorizationState;

int main()
{
    td::ClientManager client;
    auto client_id = client.create_client_id();

    client.send(client_id, 1, make_object<getMe>());
    while (true)
    {
        auto response = client.receive(10.0);
        if (!response.object) continue;
        if (response.request_id == 0)
        {
            if (response.object -> get_id() == updateAuthorizationState::ID)
            {
                // получаем объект конкретного типа. теперь updateAuthState это умный указатель
                auto updateAuthState = td::td_api::move_object_as<updateAuthorizationState>(std::move(response.object));
                if (updateAuthState -> authorization_state_)
                {
                    switch (updateAuthState -> authorization_state_ -> get_id())
                    {
                    case td::td_api::authorizationStateWaitTdlibParameters::ID:
                        {
                            auto parametrs = make_object<td::td_api::setTdlibParameters>();
                            parametrs -> database_directory_ = "./dataSession";
                            parametrs -> use_message_database_ = true;
                            parametrs -> use_secret_chats_ = true;
                            parametrs -> system_language_code_ = "en";
                            parametrs -> device_model_ = "PC";
                            parametrs -> system_version_ = "TGCleaner";
                            parametrs -> application_version_ = "1.0";
                            std::cout << "Input api_id: ";
                            std::cin >> parametrs -> api_id_;
                            std::cout << "\nInput api_hash: ";
                            std::cin >> parametrs -> api_hash_;
                            client.send(client_id, 1, std::move(parametrs));
                            break;
                        }
                    case td::td_api::authorizationStateWaitPhoneNumber::ID:
                        {
                            std::string phoneNumber;
                            std::cout << "\nInput phone number: ";
                            std::cin >> phoneNumber;
                            client.send(client_id, 1, make_object<td::td_api::setAuthenticationPhoneNumber>(phoneNumber));
                            break;
                        }
                    case td::td_api::authorizationStateWaitCode::ID:
                        {
                            std::string code;
                            std::cout << "\nPaste the code: ";
                            std::cin >> code;
                            client.send(client_id, 1, make_object<td::td_api::checkAuthenticationCode>());
                            break;
                        }
                    case td::td_api::authorizationStateReady::ID:
                        std::cout << "\nAuthorization completed!" << std::endl;
                        break;
                    case td::td_api::authorizationStateWaitPassword::ID:
                        {
                            std::string _2fa;
                            std::cout << "\n2fa password: ";
                            std::cin >> _2fa;
                            client.send(client_id, 1, make_object<td::td_api::checkAuthenticationPassword>());
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}