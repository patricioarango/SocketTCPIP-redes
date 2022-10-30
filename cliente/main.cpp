#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <time.h>

using namespace std;

bool listoParaEnviar;
bool listoParaRecibirArchivo;
time_t tiempoInicial,tiempoFinal;
bool listoParaTrabajar = true;
int MostrarMenu()
{
    cout << "Con menu en pantalla se inicia el reloj de inactividad: máx 2 min. " <<endl;
    tiempoInicial = time(NULL);
    int contador = 0;
    int menu = 0;
    cout << "**MENU: **" <<endl;
    cout << "1 - Realizar Operación" <<endl;
    cout << "2 - Ver Archivo de Logs" <<endl;
    cout << "3 - Cerrar Conexión" <<endl;
    cout << "Ingrese una opción: " <<endl;
    cin>>menu;
    return menu;
}

class Cliente{
public:
    int sock,connectRes;
    Cliente(){ }
    int Conectar()
    {
        int conexion = -1;
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            cerr << "No se puede crear el socket. Abortar." << endl;
        }

        cout << "Ingrese el puerto a Conectarse:"<<endl;
        int port;
        string ipAddress = "127.0.0.1";
        cin>>port;

        sockaddr_in hint;
        hint.sin_family = AF_INET;
        hint.sin_port = htons(port);
        inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

        //conectamos con el socket del servidor
        connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
        if (connectRes == -1)
        {
            cerr << "No se puede conectar el socket. Abortar." << endl;
            conexion = -1;

       }else{
            cout << "/*/*/*Cliente conectado*/*/*/"<<endl;
            conexion = 1;
        }
        return conexion;
    }

    void Inicio()
    {
        listoParaEnviar = true;
        listoParaRecibirArchivo = true;
        int menu_elegido = MostrarMenu();
        tiempoFinal = time(NULL);
        if((tiempoFinal - tiempoInicial) >= 180)
        {
            cout<<"El tiempo transcurrido fue mayor al autorizado. Cliente desconectado." <<endl;
            menu_elegido = 4;
        }
        switch(menu_elegido)
        {
            case 1:
                while(listoParaEnviar){
                    this->EnviarOperacion();
                }
                break;
            case 2:
                while(listoParaRecibirArchivo){
                    this->PedirArchivoLog();
                }
                break;
            case 3:
                listoParaEnviar = false;
                this->CerrarConexion();
                break;
            case 4:
                this->DesconectarConexionPorInactividad();
                break;
        }
    }

    void Enviar(string userInput)
    {
        int sendRes = send(this->sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendRes == -1)
        {
            cerr << "No se pudo conectar con el servidor. \r\n";
        }
    }

    bool AnalizarMensaje(string mensaje)
    {
        bool res = true;
        string comando = mensaje.substr(0, 6);
        if(comando == "volver")
        {
            listoParaEnviar = false;
            this->Inicio();
        }
        if(mensaje.size() >= 19 || mensaje.size() < 1)
        {
            cerr<<"La operación debe tener entre 1 y 20 caracteres"<<endl;
            res = false;
        }
        return res;
    }

    bool EnviarOperacion()
    {
        string comando = "math";
        string userInput;
        cout << "> ";
        getline(cin, userInput);
        bool analizadoOk = AnalizarMensaje(userInput);
        string mensaje = comando + userInput;
        if(analizadoOk)
        {
            Enviar(mensaje);
            Recibir();
        }
        return analizadoOk;
    }

    void PedirArchivoLog()
    {
        string comando = "file";
        this->Enviar(comando);
        while(listoParaRecibirArchivo)
        {
            string rev = Recibir();
            if(strcmp(rev.c_str(), "endoffile") == 0)
            {
                listoParaEnviar = false;
                listoParaRecibirArchivo = false;
                break;
            }
        }
        this->Inicio();
    }

    string Recibir()
    {
        char buf[4096];
        memset(buf, 0, 4096);
        int bytesReceived = recv(this->sock, buf, 4096, 0);
        if (bytesReceived == -1)
        {
            cout << "There was an error getting response from server\r\n";
        } else {
             cout << "ServidorDice> " << string(buf, bytesReceived) << "\r\n";
        }
        return string(buf, bytesReceived);
    }

    void CerrarConexion()
    {
        this->Enviar("clos");
        //this->Recibir();
        listoParaTrabajar = false;
        listoParaRecibirArchivo = false;
    }

    void DesconectarConexionPorInactividad()
    {
        this->Enviar("desc");
        this->Recibir();
        listoParaRecibirArchivo = false;
        listoParaTrabajar = false;
    }
};

int main()
{
    Cliente *cliente = new Cliente();
    int conexion = cliente->Conectar();
    if(conexion < 0)
    {
      return -1;
    }
    while(listoParaTrabajar)
    {

        cliente->Inicio();
    }
    return 0;
}
