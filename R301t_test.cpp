#include <unistd.h>

#include "./R301t_test.hpp"

// Compilacion del programa:
// g++ src/R301t_uart.cpp -o R301t -lwiringPi

/**
 * @brief Constructor de un nuevo objeto BFC350
 */
R301t::R301t()
{
}

/**
 * @brief Destructor de un objeto R301t
 */
R301t::~R301t()
{
}

/**
 * @brief Inicializa el dispositivo R301t
 *
 * @return int
 */
int R301t::start()
{
    unsigned char id[2] = {0x00, 0x01};
    unsigned short idToEnroll = (id[0] << 8) | id[1];

    if (wiringPiSetupGpio() == -1)
    {
    }
    else
    {
        pinMode(4, INPUT);
    }

    std::cout << "Iniciando proceso de enroll" << std::endl;
    while(digitalRead(23));
    enrollFinger(idToEnroll);
    std::cout << "Enroll finalizado" << std::endl;
    memset(id, 0, sizeof(id));
    while(digitalRead(23));
    searchFinger(id);
    sleep(3);
    memset(id, 0, sizeof(id));
    while(digitalRead(23));
    searchFinger(id);
    sleep(3);
    std::cout << "Borrando huellas" << std::endl;
    deleteAllFingers();
    std::cout << "Huellas borradas" << std::endl;
    id[0] = 0x00;
    id[1] = 0x05;
    sendTemplate(id);
    std::cout << "Template enviado" << std::endl;
    memset(id, 0, sizeof(id));
    while(digitalRead(23));
    searchFinger(id);
    sleep(3);
    memset(id, 0, sizeof(id));
    while(digitalRead(23));
    searchFinger(id);
    sleep(3);
    std::cout << "Test finalizado" << std::endl;

    
    // std::cout << digitalRead(23) << std::endl;
    // while (digitalRead(23))
    // {
    // }
    // std::cout << digitalRead(23) << std::endl;
    // enrollFinger(idToEnroll);
    // getTemplate(idToEnroll);

    // for(int i = 0; i<139; i++)
    //     cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(packet1[i]);
    // std::cout << std::endl;
    // std::cout << std::endl;
    // for(int i = 0; i<139; i++)
    //     cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(packet2[i]);
    // std::cout << std::endl;
    // std::cout << std::endl;
    // for(int i = 0; i<139; i++)
    //     cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(packet3[i]);
    // std::cout << std::endl;
    // std::cout << std::endl;
    // for(int i = 0; i<139; i++)
    //     cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(packet4[i]);
    // std::cout << std::endl;
    // std::cout << std::endl;

    // sendAndRecieveCmd(DownChar, sizeof(DownChar), buff_data, false);
    // std::cout << "\nDownChar sent" << std::endl;
    // sendCommand(packet1, 139);
    // std::cout << "\nPacket 1 sent" << std::endl;
    // sendCommand(packet2, 139);
    // std::cout << "\nPacket 2 sent" << std::endl;
    // sendCommand(packet3, 139);
    // std::cout << "\nPacket 3 sent" << std::endl;
    // sendCommand(packet4, 139);
    // std::cout << "\nPacket 4 sent" << std::endl;
    // id[1] = 0x51;
    // Store[11] = id[0];
    // Store[12] = id[1];
    // std::cout << "\nTrying to store template" << std::endl;
    // sendAndRecieveCmd(Store, sizeof(Store), buff_data, false);
    // std::cout << "\nTemplate stored" << std::endl;

    
    //deleteAllFingers();
    return 0;
}

/**
 * @brief Busca una huella en la memoria del dispositivo
 *
 * @param memFound Memoria en donde se ubica la huella encontrada
 *
 * @return Retorna el valor de confirmacion, 0 exito, los demas son fallos
 */
int R301t::searchFinger(unsigned char memFound[2])
{
    int confirmationCode = 1;
    unsigned char buff_data[256];
    char cmd_char[4];

    memset(cmd_char, 0, sizeof(cmd_char));
    if (!sendAndRecieveCmd(GenImg, sizeof(GenImg),
                           buff_data, false)) // Generar imagen de huella
    {
        Img2Tz[10] = static_cast<unsigned char>(1);
        if (!sendAndRecieveCmd(Img2Tz, sizeof(Img2Tz),
                               buff_data, false)) // Guardar imagen en buffer
        {
            confirmationCode = sendAndRecieveCmd(Search, sizeof(Search),
                                                 buff_data, false); // Buscar buffer en memoria
            if (confirmationCode == 0)                       // Respuesta cuando encontro huella
            {
                memFound[0] = buff_data[0];
                memFound[1] = buff_data[1];

                std::cout << "Huella encontrada en posicion: " << static_cast<int>((memFound[0] << 8) | memFound[1]) << std::endl;

            }
            else if (confirmationCode == 9)
            { // Respuesta cuando no coincide
                std::cout << "Huella no encontrada" << std::endl;
            }
            else
            {
                std::cout << "Error al buscar huella" << std::endl;
            }
        }
        else
        {
            std::cout << "Error al guardar imagen en buffer" << std::endl;
        }
    }
    else
    {
        std::cout << "Error al buscar huella" << std::endl;
    }
    return confirmationCode;
}

/**
 * @brief Elimina todas las huellas guardadas la memoria del dispositivo
 */
void R301t::deleteAllFingers()
{
    unsigned char buff_data[256];
    if (!sendAndRecieveCmd(Empty, sizeof(Empty), buff_data, false))
        std::cout << "Todos los templates de huellas han sido borrados" << std::endl;
    else
        std::cout << "Error al borrar todos los templates de huellas" << std::endl;

}

/**
 * @brief Calcula el CheckSum de buff
 *
 * @param buff Cadena a la cual calcular checksum
 * @param ln Tamaño de la cadena total
 * @return Valor del checksum calculado
 */
unsigned short R301t::calculateCheckSum(unsigned char *buff, int ln)
{
    // ln es la longitud de la cadena sin contar el CheckSum
    unsigned short sum = 0;
    for (int i = 6; i < ln; i++)
    {
        sum += buff[i];
    }
    buff[ln] = (sum >> 8) & 0xFF;
    buff[ln + 1] = sum & 0xFF;
    return sum;
}

/**
 * @brief Parsea el comando recibido por el dispositivo.
 *
 * @param buff Cadena a parsear
 * @param ln Largo de cadena
 * @param data Cadena en donde se guardan los datos que enviaron
 *
 * @return codigo de confirmacion
 */
int R301t::parseCmd(unsigned char buff[1024], int ln, unsigned char data[1024], bool doubleAnswer)
{
    int data_ln;
    int ret;
    unsigned short checkSum, aux;
    /*Los primeros 6 bytes son iguales siempre*/
    if ((buff[0] != HEADER_0 || buff[1] != HEADER_1 || buff[2] != ADDER_0 || buff[3] != ADDER_1 || buff[4] != ADDER_2 ||
        buff[5] != ADDER_3 || buff[6] != PID_ACK_PACKET) && !doubleAnswer)
    {
        cerr << "\nError al parsear [HEADERS]" << endl;
        return 1;
    }
    if ((buff[8] + 0x09) != ln && !doubleAnswer)
    {
        cerr << "\nError de tamaño de buffer" << endl;
        return 1;
    }

    /*El tamaño de la data, es lo que me dice - el checksum*/
    data_ln = buff[8] - 0x03;

    if (data_ln != 0x00)
    {
        for (int i = 0; i < data_ln; i++)
        {
            data[i] = buff[i + 10];
        }
    }
    checkSum = calculateCheckSum(buff, ln - 2);
    if ((buff[ln - 2] != ((checkSum >> 8) & 0xFF)) || (buff[ln - 1] != (checkSum & 0xFF)))
    {
        cout << "Error CheckSum" << endl;
    }

    return buff[9];
}

/**
 * @brief Envia el comando pasado como parametro y espera
 * recibir comando de respuesta
 *
 * @param cmd Cadena que se quiere enviar
 * @param ln Longitud de cadena
 * @param data Cadena en donde se guardan los datos que enviaron
 *
 * @return Codigo de confirmacion
 */
int R301t::sendAndRecieveCmd(unsigned char cmd[256], int ln, unsigned char data[1024], bool doubleAnswer)
{
    int serial_port, j = 0, resp_length = 0;
    unsigned char buffer[1024], confirmationCode;
    /*Open serial port serial0 with 57600 baudrate*/
    if ((serial_port = serialOpen("/dev/serial0", 57600)) < 0)
    {
        cerr << "No se pudo abrir el puerto serie." << endl;
        return 1;
    }
    /*Calcule the verification sum and insert in cmd*/
    calculateCheckSum(cmd, ln - 2);

    /*Send char into a finger sensor*/
    std::cout << "\nHost:";
    for (int i = 0; i < ln; i++)
    {
        serialPutchar(serial_port, cmd[i]);
        cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(cmd[i]);
    }
    std::cout << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    while (serialDataAvail(serial_port) == 0);

    /*Medicion de tiempo*/
    // auto finish = std::chrono::high_resolution_clock::now();
    // auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
    // sprintf( log_buffer, "%s<< Tiempo de respuesta Sensor= %lld uS >>", R301t_TAG, elapsed.count());
    // log.grabar( log_buffer, Debug_Log );

    /*Recieve data from finger sensor*/
    std::cout << "\nSensor:";
    while (serialDataAvail(serial_port) > 0)
    {
        buffer[j] = serialGetchar(serial_port);
        cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(buffer[j]);
        resp_length = j + 1;
        j++;
        usleep(1000);
    }
    std::cout << std::endl;

    confirmationCode = parseCmd(buffer, j, data, false);
    switch (confirmationCode)
    {
    case 0x00:
        sprintf(log_buffer, "%sEjecucion del comando satisfactoria", R301t_TAG);
        break;
    case 0x01:
        sprintf(log_buffer, "%sError al enviar el paquete", R301t_TAG);
        break;
    case 0x02:
        sprintf(log_buffer, "%sDedo no presentado en el sensor", R301t_TAG);
        break;
    case 0x03:
        sprintf(log_buffer, "%sError Enroll Huella", R301t_TAG);
        break;
    case 0x06:
        sprintf(log_buffer, "%sError al generar char por over-disorderly en imagen de huella", R301t_TAG);
        break;
    case 0x07:
        sprintf(log_buffer, "%sError al generar char por lackness o imagen muy pequeña", R301t_TAG);
        break;
    case 0x08:
        sprintf(log_buffer, "%sHuellas no coinciden", R301t_TAG);
        break;
    case 0x09:
        sprintf(log_buffer, "%sNo se pudo encontrar huella que coincida", R301t_TAG);
        break;
    case 0x0A:
        sprintf(log_buffer, "%sError al combinar Chars", R301t_TAG);
        break;
    case 0x0B:
        sprintf(log_buffer, "%sNumero de PageID mas grande que library", R301t_TAG);
        break;
    case 0x0C:
        sprintf(log_buffer, "%sError leyendo Template o Template invalido", R301t_TAG);
        break;
    case 0x0D:
        sprintf(log_buffer, "%sError subiendo Template", R301t_TAG);
        break;
    case 0x0E:
        sprintf(log_buffer, "%sModulo no pudo los paquetes de datos", R301t_TAG);
        break;
    case 0x0F:
        sprintf(log_buffer, "%sEror subiendo imagen", R301t_TAG);
        break;
    case 0x10:
        sprintf(log_buffer, "%sError eliminando Template", R301t_TAG);
        break;
    case 0x11:
        sprintf(log_buffer, "%sError limpiando libreria de huellas", R301t_TAG);
        break;
    case 0x13:
        sprintf(log_buffer, "%sContrasena incorrecta", R301t_TAG);
        break;
    case 0x15:
        sprintf(log_buffer, "%sError generando imagen por falta de una imagen primaria valida", R301t_TAG);
        break;
    case 0x18:
        sprintf(log_buffer, "%sError escribiendo memoria flash", R301t_TAG);
        break;
    case 0x19:
        sprintf(log_buffer, "%sError de no definicion", R301t_TAG);
        break;
    case 0x1A:
        sprintf(log_buffer, "%sNumero de registro invalido", R301t_TAG);
        break;
    case 0x1B:
        sprintf(log_buffer, "%sConfiguracion de registro incorrecta", R301t_TAG);
        break;
    case 0x1C:
        sprintf(log_buffer, "%sNumero de pagina de NotePad incorrecta", R301t_TAG);
        break;
    case 0x1D:
        sprintf(log_buffer, "%sError para manejar la comunicacion del puerto", R301t_TAG);
        break;
    default:
        sprintf(log_buffer, "%sNo se encontro ninguna coincidencia CC", R301t_TAG);
        break;
    }

    if(doubleAnswer && confirmationCode == 0x00){
        while (serialDataAvail(serial_port) == 0);
        /*Recieve data from finger sensor*/
        j = 0;
        std::cout << "\nSensor sending template data:";
        while (serialDataAvail(serial_port) > 0)
        {
            buffer[j] = serialGetchar(serial_port);
            if(j < 139)
                packet1[j] = buffer[j];
            else if (j < 278)
                packet2[j-139] = buffer[j];
            else if (j < 417)
                packet3[j-278] = buffer[j];
            else if (j < 556)
                packet4[j-417] = buffer[j];
            cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(buffer[j]);
            resp_length = j + 1;
            j++;
            usleep(1000);
        }

        // while (serialDataAvail(serial_port) == 0);

        // std::cout << "\nSensor sending template data:";
        // while (serialDataAvail(serial_port) > 0)
        // {
        //     buffer[j] = serialGetchar(serial_port);
        //     cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(buffer[j]);
        //     resp_length = j + 1;
        //     j++;
        //     usleep(1000);
        // }

        confirmationCode = parseCmd(buffer, j, data, true);
    }

    serialClose(serial_port);
    return confirmationCode;
}

int R301t::receiveCommand(unsigned char data[1024])
{
    int serial_port, j = 0, resp_length = 0;
    unsigned char buffer[256], confirmationCode;
    /*Open serial port serial0 with 57600 baudrate*/
    if ((serial_port = serialOpen("/dev/serial0", 57600)) < 0)
    {
        cerr << "No se pudo abrir el puerto serie." << endl;
        return 1;
    }

    while (serialDataAvail(serial_port) == 0)
    {
        std::cout << "Esperando respuesta del sensor" << std::endl;
        usleep(1000);
    }

    /*Recieve data from finger sensor*/
    std::cout << "\nSensor:";
    while (serialDataAvail(serial_port) > 0)
    {
        buffer[j] = serialGetchar(serial_port);
        cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(buffer[j]);
        resp_length = j + 1;
        j++;
        usleep(1000);
    }

    confirmationCode = parseCmd(buffer, j, data, false);
    return confirmationCode;
}

void R301t::sendCommand(unsigned char cmd[256], int ln)
{
    int serial_port, j = 0, resp_length = 0;
    unsigned char buffer[256], confirmationCode;
    /*Open serial port serial0 with 57600 baudrate*/
    if ((serial_port = serialOpen("/dev/serial0", 57600)) < 0)
    {
        cerr << "No se pudo abrir el puerto serie." << endl;
        //return 1;
    }

    /*Calcule the verification sum and insert in cmd*/
    calculateCheckSum(cmd, ln - 2);


    /*Send char into a finger sensor*/
    std::cout << "\nHost:";
    for (int i = 0; i < ln; i++)
    {
        serialPutchar(serial_port, cmd[i]);
        cout << " " << hex << setw(2) << setfill('0') << uppercase << static_cast<int>(cmd[i]);
    }

    //confirmationCode = parseCmd(buffer, j, data, false);
    //return confirmationCode;
}

/**
 * @brief Realiza enroll de un dedo nuevo en un espacio de memoria especificado
 *
 * @param memoryLocate parametro de memoria donde se desea guardar la huella,
 * deben ser 2 bytes
 *
 * @return Codigo de confirmacion
 */
void R301t::enrollFinger(unsigned short memoryLocate)
{
    unsigned char buff_data[256];
    int confirmationCode;
    bool exitoF = false; // Flag que me permite determinar que se realizo con exito el ciclo

    std::cout << "Enroll de huella" << std::endl;

    /*Generar 2 imagenes, guardarlo en los 2 buffers*/
    for (int i = 1; i < 3; i++)
    {
        confirmationCode = sendAndRecieveCmd(GenImg, sizeof(GenImg),
                                             buff_data, false); // Gen finger image
        if (!confirmationCode)
        {
            Img2Tz[10] = static_cast<unsigned char>(i);
            confirmationCode = sendAndRecieveCmd(Img2Tz, sizeof(Img2Tz),
                                                 buff_data, false); // Save image i in bufferi
            if (confirmationCode)
                break;
            if (!confirmationCode && (i == 2))
                exitoF = true;
        }
    }
    if (exitoF)
    {
        /*Registrar los dos buffers como un template*/
        if (sendAndRecieveCmd(RegModel, sizeof(RegModel), buff_data, false) == 0)
        {
            Store[11] = (memoryLocate >> 8) & 0xFF;
            Store[12] = memoryLocate;

            if (sendAndRecieveCmd(UpChar, sizeof(UpChar), buff_data, true) == 0) // Sube template de los buffers a PC
            {
                cout << "\nHuella subida correctamente" << endl;
            }
            else
            {
                cout << "\nError al subir la huella" << endl;
            }

            /*Guardar el template en la memoria indicada*/
            if (sendAndRecieveCmd(Store, sizeof(Store), buff_data, false) == 0)
            {
                cout << "Huella almacenada correctamente en PageID: 0x" << hex << setw(2) << setfill('0') << uppercase
                     << static_cast<int>(Store[11]) << hex << setw(2) << setfill('0') << uppercase
                     << static_cast<int>(Store[12]) << endl;

                sleep(5);
            }
        }
    }
}

int R301t::sendTemplate(unsigned char id[2])
{ 
    
    sendAndRecieveCmd(DownChar, sizeof(DownChar), buff_data, false);
    std::cout << "\nDownChar sent" << std::endl;
    sendCommand(packet1, 139);
    std::cout << "\nPacket 1 sent" << std::endl;
    sendCommand(packet2, 139);
    std::cout << "\nPacket 2 sent" << std::endl;
    sendCommand(packet3, 139);
    std::cout << "\nPacket 3 sent" << std::endl;
    sendCommand(packet4, 139);
    std::cout << "\nPacket 4 sent" << std::endl;
    Store[11] = id[0];
    Store[12] = id[1];
    std::cout << "\nTrying to store template" << std::endl;
    sendAndRecieveCmd(Store, sizeof(Store), buff_data, false);
    std::cout << "\nTemplate stored" << std::endl;
    
    return 0;
}

int R301t::getTemplate(unsigned short idToGet)
{
    LoadChar[11] = (idToGet >> 8) & 0xFF;
    LoadChar[12] = idToGet;

    if(!sendAndRecieveCmd(LoadChar, sizeof(LoadChar), buff_data, false))
    {
        std::cout << "\nHuella cargada en buffer correctamente" << std::endl;

        if(!sendAndRecieveCmd(UpChar, sizeof(UpChar), buff_data, true))
        {
            std::cout << "\nHuella enviada correctamente" << std::endl;
        }
        else
        {
            std::cout << "\nError al enviar la huella" << std::endl;
        }
    }
    else
    {
        std::cout << "\nError al cargar la huella en buffer" << std::endl;
    }
        
    return 0;
}

int main()
{
    R301t r301t;
    return r301t.start();
}
