#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <fstream>
#include <regex>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
bool funcionar = false;

struct analisisSignos{
  int cantidadSignos = 0;
  int posicionSigno = 0;
  int tipoOperacion = 0; //tipoOperacion suma = 1 resta = 0 // factorial = 2 // potencia = 3 // division = 4 // multi = 5 //
};

char signosAdmitidos[] = {'+','-','*','/','^','!'};
char numerosAdmitidos[] = {'0','1','2','3','4','5','6','7','8','9'};
char soloCaracteresAdmitidos[] = {'+','-','*','/','^','!','0','1','2','3','4','5','6','7','8','9'};

void EscribirArchivoLog(string log)
{
    time_t curr_time;
	tm * curr_tm;
	char date_string[100];
	char time_string[100];
	time(&curr_time);
	curr_tm = localtime(&curr_time);
	strftime(date_string, 50, "%Y-%m-%d ", curr_tm);
	strftime(time_string, 50, "%T:", curr_tm);
    ofstream MyFile("server.log", ios::out | ios::app);
    MyFile << date_string << time_string << log <<endl;
    MyFile.close();
}

class Servidor{
public:
    socklen_t server, client;
    sockaddr_in serverAddr, clientAddr;
    char buffer[4096];
    int clientSocket;
    int PUERTO = 5000;
    int servidor;
    Servidor() {
        servidor = socket(AF_INET, SOCK_STREAM, 0);
        //AF_INET => tipo dominio ipv4; SOCK_STREAM=> tipo de socket stream; 0 protocolo a utilizar
        if (servidor == -1)
        {
            cerr << "No se puede crear el socket. Abortar." << endl;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PUERTO);
        inet_pton(AF_INET, "0.0.0.0", &serverAddr.sin_addr);
        string msj = " /*INICIA SERVIDOR*/ ";
        cout << msj << endl;
        EscribirArchivoLog(msj);
        bind(servidor, (sockaddr*)&serverAddr, sizeof(serverAddr));
    }

    bool Conectar()
    {
        int escuchar = listen(this->servidor, 0);
        string msj2 = " Socket creado en Puerto: " + std::to_string(PUERTO);
        cout << msj2 << endl;
        EscribirArchivoLog(msj2);
        socklen_t clientSize = sizeof(client);
        clientSocket = accept(servidor, (sockaddr*)&clientAddr, &clientSize);

        char host[NI_MAXHOST];      // Client's remote name
        char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on
        if (getnameinfo((sockaddr*)&clientAddr, sizeof(clientAddr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
        {
            string msj3 = "**CLIENTE CONECTADO***";
            EscribirArchivoLog(msj3);
            cout<< msj3<< endl;
        }
        if(clientSocket < 0)
        {
            return false;
        } else {
            return true;
        }
    }

    int Recibir()
    {
        char buf[4096];
        memset(buf, 0, 4096);
        int bytesReceived = recv(this->clientSocket, buf, sizeof(buf), 0);
        std::string str = buf;
        cout << "ClienteDice> "<< string(buf, 0, bytesReceived) << endl;
        memset(buf, 0, sizeof(buf));
        string mensaje = str;
            if(mensaje != ""){

            int TareaARealizar = this->AnalizarComando(mensaje);
                switch(TareaARealizar)
                {
                    case 1:
                        this->RealizarOperacion(mensaje);
                        break;
                    case 2:
                        this->EnviarArchivo();
                        break;
                    case 3:
                        this->CerrarSocket();
                        this->Conectar();
                        break;
                    case 4:
                        string msj7 = "**CLIENTE DESCONECTADO POR INACTIVIDAD**";
                        this->Enviar(msj7);
                        EscribirArchivoLog(msj7);
                        cout<<msj7<<endl;
                        this->CerrarSocket();
                        this->Conectar();
                        break;
                }
            }
            return bytesReceived;
    }

    int AnalizarComando(string mensaje)
    {
        string comando = mensaje.substr(0, 4);
        int res = 0;
        if(comando == "math")
        {
            res = 1;
        }
        if(comando == "file")
        {
            res = 2;
        }
        if(comando == "clos")
        {
            res = 3;
        }
        if(comando == "desc")
        {
            res = 4;
        }
        return res;
    }

    void EnviarErrorCaracterNoContemplado(string caracter)
    {
        string mensaje = "No se pudo realizar. Caracter no contemplado: " + caracter;
        Enviar(mensaje);
    }

    void EnviarOperacionMalFormada(string parte_mal_formada)
    {
        string mensaje = "No se pudo realizar. Operación mal formada: " + parte_mal_formada;
        Enviar(mensaje);
    }

    void EnviarResultado(int resultado)
    {
        string mensaje = "Resultado: " + std::to_string(resultado);
        Enviar(mensaje);
    }


    void Enviar(string mensaje)
    {
        send(this->clientSocket, mensaje.c_str(), mensaje.size() + 1, 0);
        cout << "Respuesta enviada!" << endl;
    }

    void RealizarOperacion(string mensaje)
    {
        validarOperacion(mensaje);
    }

    void validarOperacion(string mensaje)
    {
        string operacion = mensaje.substr(4, mensaje.length());
        //buscamos si tiene letras
        regex reg("[abcdefghijklmnopqrstuvwxyz#@$%&()?]"); //agregar mayus y otros signos no aceptados
        if(regex_search(operacion, reg)){
            int posLetra = BuscarCaracteresInvalidos(operacion);
            EnviarErrorCaracterNoContemplado(operacion.substr(posLetra, 1));
        } else {
            regex simbolo("[-|+|!|*|/|^]");
            if(regex_search(operacion, simbolo)){
                analisisSignos analisis = AnalisisSignos(operacion);
                int validacion = 0;
                if(analisis.tipoOperacion == 3)
                {
                    validacion = ValidacionPotencia(operacion, analisis);
                }
                if(analisis.tipoOperacion == 2)
                {
                    validacion = ValidacionFactorial(operacion, analisis);
                }
                if(analisis.tipoOperacion == 4)
                {
                    validacion = ValidacionDivision(operacion, analisis);
                }
                if(analisis.tipoOperacion == 5)
                {
                    validacion = ValidacionMultiplicacion(operacion, analisis);
                }
                if(analisis.tipoOperacion == 1)
                {
                    validacion = ValidacionSumaResta(operacion, analisis);
                }
                if(analisis.tipoOperacion == 0)
                {
                    cout<<"aca"<<endl;
                    validacion = ValidacionSumaResta(operacion, analisis);
                }
                if(validacion > 0)
                {
                    ResolverOperacion(operacion, analisis);
                }
            } else {
                //no tiene letras pero tampoco simbolos // aca prueba d
                this->Enviar(operacion);
                return;
            }
        }
    }

    /** buscarPosicionLetra
    ** @param string de operación
    ** @return la primera posición de la letra encontrada
    */
    int BuscarCaracteresInvalidos(string operacion)
    {
        bool busqueda = true;
        int i=0;
        while(operacion.length() > i && busqueda) {
            if(operacion[i] != soloCaracteresAdmitidos[0]
            && operacion[i] != soloCaracteresAdmitidos[1]
            && operacion[i] != soloCaracteresAdmitidos[2]
            && operacion[i] != soloCaracteresAdmitidos[3]
            && operacion[i] != soloCaracteresAdmitidos[4]
            && operacion[i] != soloCaracteresAdmitidos[5]
            && operacion[i] != soloCaracteresAdmitidos[6]
            && operacion[i] != soloCaracteresAdmitidos[7]
            && operacion[i] != soloCaracteresAdmitidos[8]
            && operacion[i] != soloCaracteresAdmitidos[9]
            && operacion[i] != soloCaracteresAdmitidos[10]
            && operacion[i] != soloCaracteresAdmitidos[11]
            && operacion[i] != soloCaracteresAdmitidos[12]
            && operacion[i] != soloCaracteresAdmitidos[13]
            && operacion[i] != soloCaracteresAdmitidos[14]
            && operacion[i] != soloCaracteresAdmitidos[15]
            && operacion[i] != soloCaracteresAdmitidos[16]){
                busqueda = false;
            }
            i++;
        }
        return i-1;
    }

    /**
    ** AnalisisSignos
    ** @param string de operacion
    ** @return devuelve una estructura analisisSigno { int cantidadSignos, int posicionSigno }
    ** cantidad de signos: 0 y 1 son validas, mas de 1 signo es invalida
    ** posicion del signo: valido para cuando tiene 1 signo, para saber si esta bien ubicado
    ** ya que no podria estar en posicion cero
    */
    /** DeterminarOperacion
    ** @param string de operacion
    ** Si tiene * / ! pow es ese tipo de operacion. Luego se valida cada una en particular
    ** Si tiene solo + - se valida aparte puede ser +5+-9
    */
    analisisSignos AnalisisSignos(string operacion)
    {
        analisisSignos analisis;
        for(int i=0 ; i<operacion.length();i++) {
            //cantidad de signos
            for(int j=0; j<sizeof(signosAdmitidos);j++)
            {
                if(operacion[i] == signosAdmitidos[j])
                {
                    analisis.cantidadSignos++;
                    analisis.posicionSigno = i;
                }
            }
            //tipo de Operacion
            //es potencia?
            if(operacion[i] == signosAdmitidos[4])
            {
                 analisis.tipoOperacion = 3;
                 break;
            }
            //es factorial?
            if(operacion[i] == signosAdmitidos[5])
            {
                 analisis.tipoOperacion = 2;
                 break;
            }
            //es division?
            if(operacion[i] == signosAdmitidos[3])
            {
                 analisis.tipoOperacion = 4;
                 break;
            }
            //es multiplicacion?
            if(operacion[i] == signosAdmitidos[2])
            {
                 analisis.tipoOperacion = 5;
                 break;
            }
            //si es suma no entra en break
            if(operacion[i] == signosAdmitidos[0])
            {
                 analisis.tipoOperacion = 1;
            }
            //si es resta no entra en break
            if(operacion[i] == signosAdmitidos[1])
            {
                 cout<<"tipo operacion0"<<endl;
                 analisis.tipoOperacion = 0;
            }
        }
        return analisis;
    }

    /** validacionFactorial
    ** Si es factorial solo puede tener un signo !
    ** El signo no puede estar en posicion 0
    ** No puede tener caracteres despues del signo ! entonces =>
    ** el tamaño de la operacion igual a la posicion del signo
    ** @param string operacion
    ** @param analisisSignos analisis
    ** @return validado ok = 1, nok = -1
    */
    int ValidacionFactorial(string operacion,analisisSignos analisis)
    {
        int validacion = 1;
        //el signo no puede estar al inicio
        if(analisis.posicionSigno == 0)
        {
            string parte_mal_formada = operacion.substr(0,2);
            EnviarOperacionMalFormada(parte_mal_formada);
            validacion = -1;
        }
        //el signo tiene que estar al final
        if(analisis.posicionSigno != (operacion.length() - 1) && analisis.posicionSigno > 0)
        {
            string parte_mal_formada = operacion.substr((analisis.posicionSigno-1), operacion.length());
            EnviarOperacionMalFormada(parte_mal_formada);
            validacion = -1;
        }
        return validacion;
    }

    /** validacionPotencia
    ** No puede tener más de 3 signos
    ** el signo no puede estar al final
    ** el signo no puede estar al principio
    ** no puede tener otros signos que no sean + -
    ** @param string operacion
    ** @param analisisSignos analisis
    ** @return validado ok = 1, nok = -1
    */
    int ValidacionPotencia(string operacion,analisisSignos analisis)
    {
        int validacion = 1;
        //el signo no puede estar al inicio
        if(analisis.cantidadSignos == 1 && analisis.posicionSigno == 0)
        {
            string parte_mal_formada = operacion.substr(0,2);
            EnviarOperacionMalFormada(parte_mal_formada);
            validacion = -1;
        } else {
            //el signo no puede estar al final
            if(analisis.cantidadSignos == 1 && analisis.posicionSigno > 0 && (operacion.length() == (analisis.posicionSigno + 1)))
            {
                string parte_mal_formada = operacion.substr((analisis.posicionSigno-1), operacion.length());
                EnviarOperacionMalFormada(parte_mal_formada);
                validacion = -1;
            }
        }
        string base = operacion.substr(0,analisis.posicionSigno);
        string exponente = operacion.substr((analisis.posicionSigno+1),operacion.length());
        analisisSignos analisisBase = AnalisisSignos(base);
        analisisSignos analisisExponente = AnalisisSignos(exponente);
        //la base solo puede tener signos + o -
        if(analisisBase.tipoOperacion > 1)
        {
            EnviarOperacionMalFormada(base);
            validacion = -1;
        }
        //la base solo puede tener uno de esos signos
        if(analisisBase.tipoOperacion == 1 && analisisBase.cantidadSignos > 1)
        {
            EnviarOperacionMalFormada(base);
            validacion = -1;
        }
        //el signo + - de la base debe estar en posicion 0
        if(analisisBase.tipoOperacion == 1 && analisisBase.cantidadSignos == 1 && analisisBase.posicionSigno > 0)
        {
            EnviarOperacionMalFormada(base);
            validacion = -1;
        }
        //el exponente solo puede tener signos + o -
        if(analisisExponente.tipoOperacion > 1)
        {
            EnviarOperacionMalFormada(exponente);
            validacion = -1;
        }
        //el exponente solo puede tener uno de esos signos
        if(analisisExponente.tipoOperacion == 1 && analisisExponente.cantidadSignos > 1)
        {
            EnviarOperacionMalFormada(exponente);
            validacion = -1;
        }
        //el exponente + - de la base debe estar en posicion 0
        if(analisisExponente.tipoOperacion == 1 && analisisExponente.cantidadSignos == 1 && analisisExponente.posicionSigno > 0)
        {
            EnviarOperacionMalFormada(exponente);
            validacion = -1;
        }
        return validacion;
    }

    /** ValidacionDivision
    ** No puede tener más de 1 signo de /
    ** el signo no puede estar al final
    ** el signo no puede estar al principio
    ** el divisor debe ser > 0
    ** el divisor puede tener signo + y -, pero solo al inicio
    ** el dividendo puede tener signo + y -, pero solo al inicio
    ** @param string operacion
    ** @param analisisSignos analisis
    ** @return validado ok = 1, nok = -1
    */
    int ValidacionDivision(string operacion,analisisSignos analisis)
    {
        int validacion = 1;
        //el signo no puede estar al inicio
        if(analisis.cantidadSignos == 1 && analisis.posicionSigno == 0)
        {
            string parte_mal_formada = operacion.substr(0,2);
            EnviarOperacionMalFormada(parte_mal_formada);
            validacion = -1;
        } else {
            //el signo no puede estar al final
            if(analisis.cantidadSignos == 1 && analisis.posicionSigno > 0 && (operacion.length() == (analisis.posicionSigno + 1)))
            {
                string parte_mal_formada = operacion.substr((analisis.posicionSigno-1), operacion.length());
                EnviarOperacionMalFormada(parte_mal_formada);
                validacion = -1;
            }
        }

        string dividendo = operacion.substr(0,analisis.posicionSigno);
        string divisor = operacion.substr((analisis.posicionSigno+1),operacion.length());
        analisisSignos analisisDividendo = AnalisisSignos(dividendo);
        analisisSignos analisisDivisor = AnalisisSignos(divisor);
        //el dividendo no puede tener signos que no sean + o -
        if(analisisDividendo.tipoOperacion > 1)
        {
            EnviarOperacionMalFormada(dividendo);
            validacion = -1;
        }
        //si dividendo tiene signos + o - solo pueden tener 1 de ellos
        if(analisisDividendo.tipoOperacion == 1 && analisisDividendo.cantidadSignos > 1)
        {
            EnviarOperacionMalFormada(dividendo);
            validacion = -1;
        }
        //si dividendo tiene signos + o - y solo pueden tener 1 de ellos, solo puede estar en posicion 0
        if(analisisDividendo.tipoOperacion == 1 && analisisDividendo.cantidadSignos == 1 && analisisDividendo.posicionSigno > 0)
        {
            EnviarOperacionMalFormada(dividendo);
            validacion = -1;
        }
        //el divisor no puede tener signos que no sean + o -
        if(analisisDivisor.tipoOperacion > 1)
        {
            EnviarOperacionMalFormada(divisor);
            validacion = -1;
        }
        //si divisor tiene signos + o - y solo pueden tener 1 de ellos, solo puede estar en posicion 0
        if(analisisDivisor.tipoOperacion == 1 && analisisDivisor.cantidadSignos == 1 && analisisDivisor.posicionSigno > 0)
        {
            EnviarOperacionMalFormada(divisor);
            validacion = -1;
        }

        //el divisor tiene que ser mayor de cero
        if(atoi(divisor.c_str()) == 0)
        {
            EnviarOperacionMalFormada(divisor);
            validacion = -1;
        }
        return validacion;
    }

    /** ValidacionMultiplicacion
    ** No puede tener más de 1 signo de /
    ** el signo no puede estar al final
    ** el signo no puede estar al principio
    ** el multiplicando puede tener signo + y -, pero solo al inicio
    ** el multiplicador puede tener signo + y -, pero solo al inicio
    ** @param string operacion
    ** @param analisisSignos analisis
    ** @return validado ok = 1, nok = -1
    */
    int ValidacionMultiplicacion(string operacion,analisisSignos analisis)
    {
        int validacion = 1;
        //el signo no puede estar al inicio
        if(analisis.cantidadSignos == 1 && analisis.posicionSigno == 0)
        {
            string parte_mal_formada = operacion.substr(0,2);
            EnviarOperacionMalFormada(parte_mal_formada);
            validacion = -1;
        } else {
            //el signo no puede estar al final
            if(analisis.cantidadSignos == 1 && analisis.posicionSigno > 0 && (operacion.length() == (analisis.posicionSigno + 1)))
            {
                string parte_mal_formada = operacion.substr((analisis.posicionSigno-1), operacion.length());
                EnviarOperacionMalFormada(parte_mal_formada);
                validacion = -1;
            }
        }

        string multiplicando = operacion.substr(0,analisis.posicionSigno);
        string multiplicador = operacion.substr((analisis.posicionSigno+1),operacion.length());
        analisisSignos analisisMultiplicando = AnalisisSignos(multiplicando);
        analisisSignos analisisMultiplicador = AnalisisSignos(multiplicador);

        //el multiplicando no puede tener signos que no sean + o -
        if(analisisMultiplicando.tipoOperacion > 1)
        {
            EnviarOperacionMalFormada(multiplicando);
            validacion = -1;
        }
        //si multiplicando tiene signos + o - solo pueden tener 1
        if(analisisMultiplicando.tipoOperacion == 1 && analisisMultiplicando.cantidadSignos > 1)
        {
            EnviarOperacionMalFormada(multiplicando);
            validacion = -1;
        }
        //si multiplicando tiene signos + o - y solo pueden tener 1 de ellos, solo puede estar en posicion 0
        if(analisisMultiplicando.tipoOperacion == 1 && analisisMultiplicando.cantidadSignos == 1 && analisisMultiplicando.posicionSigno > 0)
        {
            EnviarOperacionMalFormada(multiplicando);
            validacion = -1;
        }
        //el multiplicador no puede tener signos que no sean + o -
        if(analisisMultiplicador.tipoOperacion > 1)
        {
            string parte_mal_formada = operacion.substr((analisis.posicionSigno),operacion.length());
            EnviarOperacionMalFormada(parte_mal_formada);
            validacion = -1;
        }
        //si multiplicando tiene signos + o - solo pueden tener 1
        if(analisisMultiplicador.tipoOperacion == 1 && analisisMultiplicador.cantidadSignos > 1)
        {
            EnviarOperacionMalFormada(multiplicador);
            validacion = -1;
        }
        //si multiplicador tiene signos + o - y solo pueden tener 1 de ellos, solo puede estar en posicion 0
        if(analisisMultiplicador.tipoOperacion == 1 && analisisMultiplicador.cantidadSignos == 1 && analisisMultiplicador.posicionSigno > 0)
        {
            EnviarOperacionMalFormada(multiplicador);
            validacion = -1;
        }
        return validacion;
    }

    /** ValidacionSumaResta
    ** como el analisis no tiene break para este signo
    ** la posicion del signo va a ser la ultima encontrada
    ** no puede tener signo como ultima posicion
    ** si tiene un signo, no puede ser en posicion inicial
    ** mas de 3 signos no puede tener
    ** si tiene 3 signos, no pueden ser consecutivos
    ** @param string operacion
    ** @param analisisSignos analisis
    ** @return validado ok = 1, nok = -1
    */
    int ValidacionSumaResta(string operacion,analisisSignos analisis)
    {
        int validacion = 1;
        cout<<"analisis.cantidadSignos"<<endl;
        cout<<analisis.cantidadSignos<<endl;
        cout<<"analisis.posicionSigno"<<endl;
        cout<<analisis.posicionSigno<<endl;
        cout<<"analisis.tipoOperacion"<<endl;
        cout<<analisis.tipoOperacion<<endl;
        string parte1 = operacion.substr(0,analisis.posicionSigno);
        string parte2 = operacion.substr((analisis.posicionSigno+1),operacion.length());
        analisisSignos analisisParte1 = AnalisisSignos(parte1);
        analisisSignos analisisParte2 = AnalisisSignos(parte2);
        //si parte1 tiene + 1 signos + o
        if(analisisParte1.cantidadSignos > 1)
        {
            EnviarOperacionMalFormada(parte1);
            validacion = -1;
        }
        //si parte1 tiene signos + o - solo pueden tener 1
        if(analisisParte1.cantidadSignos == 1 && analisisParte1.posicionSigno > 1)
        {
            EnviarOperacionMalFormada(parte1);
            validacion = -1;
        }
        //si parte2 tiene + 1 signos + o
        if(analisisParte2.cantidadSignos > 1)
        {
            EnviarOperacionMalFormada(parte2);
            validacion = -1;
        }
        //si parte2 tiene signos + o - solo pueden tener 1
        if(analisisParte2.cantidadSignos == 1 && analisisParte2.posicionSigno > 1)
        {
            EnviarOperacionMalFormada(parte2);
            validacion = -1;
        }
        return validacion;
    }
    void ResolverOperacion(string operacion,analisisSignos analisis)
    {
        int resultado;
        switch(analisis.tipoOperacion)
        {
            case 2: //FACTORIAL
            {
                int nro = atoi(operacion.substr(0, analisis.posicionSigno).c_str());
                resultado = 1;
                    if(nro > 0)
                    {
                        for(int i=1; i<=nro; i++)
                        {
                            resultado = resultado * i;
                        }
                    }
                 break;
            }
            case 3: //POTENCIA
            {
                std::string p1 = operacion.substr(0, analisis.posicionSigno);
                std::string p2 = operacion.substr((analisis.posicionSigno+1),operacion.length());
                resultado = pow(atoi(p1.c_str()),atoi(p2.c_str()));
                break;
            }
            case 4: //DIVISON
            {
                std::string p1 = operacion.substr(0, analisis.posicionSigno);
                std::string p2 = operacion.substr((analisis.posicionSigno+1),operacion.length());
                resultado = atoi(p1.c_str())  / atoi(p2.c_str());
                break;
            }
            case 5: //Multiplicacion
            {
                std::string p1 = operacion.substr(0, analisis.posicionSigno);
                std::string p2 = operacion.substr((analisis.posicionSigno+1),operacion.length());
                resultado = atoi(p1.c_str())  * atoi(p2.c_str());
                break;
            }
            case 1: //Suma
            {
                std::string p1 = operacion.substr(0, analisis.posicionSigno);
                std::string p2 = operacion.substr((analisis.posicionSigno+1),operacion.length());
                resultado = atoi(p1.c_str())  + atoi(p2.c_str());
                break;
            }
            case 0: //Resta
            {
                std::string p1 = operacion.substr(0, analisis.posicionSigno);
                std::string p2 = operacion.substr((analisis.posicionSigno+1),operacion.length());
                resultado = atoi(p1.c_str())  - atoi(p2.c_str());
                break;
            }
        }
        EnviarResultado(resultado);
    }

    void EnviarArchivo()
    {
        std::ifstream file("server.log");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                Enviar(line);
            }
            file.close();
        }
        Enviar("endoffile");
    }

    void CerrarSocket()
    {
        cout << "Socket cerrado, cliente desconectado." << endl;
        EscribirArchivoLog("**CLIENTE DESCONECTADO**");
        close(this->clientSocket);
    }
};

int main()
{
    Servidor *Server = new Servidor();
    while(Server->Conectar())
    {
        while(Server->Recibir())
        {
            Server->Recibir();
        }
    }
}
