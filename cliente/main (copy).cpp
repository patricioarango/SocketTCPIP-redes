#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

void cerrar_conexion(int sock)
{
    string userInput = "mensaje fake";
    int sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
}

void realizar_operacion()
{
    cout << "Ingrese la Operación ";
}

void mostrar_menu(int sock)
{
    int menu;
    cout << "Ingrese una opción:" <<endl;
    cout << "1 - Realizar Operación" <<endl;
    cout << "2 - Ver Archivo de Logs" <<endl;
    cout << "3 - Cerrar Conexión" <<endl;
    cin>>menu;
    switch(menu)
    {
        case 1:
            realizar_operacion();
            break;
        case 2:
            realizar_operacion();
            break;
        case 3:
            cerrar_conexion(sock);
            break;
    }
}



int conectar_socket()
{
    //	Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return 1;
    }

    //	Create a hint structure for the server we're connecting with
    int port = 54000;
    string ipAddress = "127.0.0.1";

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    //	Connect to the server on the socket
    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1)
    {
       // return 1;
    }
    return sock;
}



int main()
{

    int sock = conectar_socket();
    //	While loop:
    char buf[4096];
    string userInput;


    while(true) {
        mostrar_menu(sock);

        //		Enter lines of text
        cout << "> ";
        getline(cin, userInput);

        //		Send to server
        int sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendRes == -1)
        {
            cout << "Could not send to server! Whoops!\r\n";
            continue;
        }

        //		Wait for response
        memset(buf, 0, 4096);
        int bytesReceived = recv(sock, buf, 4096, 0);
        if (bytesReceived == -1)
        {
            cout << "There was an error getting response from server\r\n";
        }
        else
        {
            //		Display response
            cout << "SERVER> " << string(buf, bytesReceived) << "\r\n";
        }
    }

    //	Close the socket
    close(sock);

    return 0;
}
