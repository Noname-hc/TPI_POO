#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>   // memset, strerror
#include <errno.h>   // errno
#include <sys/select.h>
#include <sys/time.h>
#include <map>

#include "Serial_Com.h"


    void Serial_Com::SetCadena(const std::string& nuevaCadena) {
        cadena = nuevaCadena;
    }

    static speed_t baud_map(int baud) {
        switch (baud) {
            case 9600: return B9600;
            case 19200: return B19200;
            case 38400: return B38400;
            case 57600: return B57600;
            case 115200: return B115200;
            default: return B19200; // valor por defecto
        }
    }

    Serial_Com::Serial_Com() {
        this->cadena="";
        this->fd = -1;
    }

    Serial_Com::~Serial_Com() {
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
    }

    void Serial_Com::T_R_Init(int Brate, uint8_t Stop_bits, std::string Port) {
        // Abrimos el puerto en lectura/escritura, no non-blocking para que select funcione bien
        fd = open(Port.c_str(), O_RDWR | O_NOCTTY);
        if (fd < 0) {
            std::cerr << "Error abriendo puerto: " << strerror(errno) << std::endl;
            return;
        }

        struct termios options;
        memset(&options, 0, sizeof(options));

        if (tcgetattr(fd, &options) == -1) {
            std::cerr << "Error obteniendo configuración: " << strerror(errno) << std::endl;
            close(fd); fd = -1; return;
        }

        speed_t spd = baud_map(Brate);
        cfsetispeed(&options, spd);
        cfsetospeed(&options, spd);

        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;     // 8 bits
        options.c_cflag |= CREAD;   // habilitar lectura
        options.c_cflag |= CLOCAL;  // ignorar control de modem
        options.c_cflag &= ~PARENB; // sin paridad

        if (Stop_bits == 1) options.c_cflag &= ~CSTOPB;
        else options.c_cflag |= CSTOPB;

        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // raw mode
        options.c_iflag &= ~(IXON | IXOFF | IXANY); // deshabilitar flow control por software
        options.c_oflag &= ~OPOST; // raw output

        options.c_cc[VMIN] = 0;   // no bloquear por cantidad de bytes
        options.c_cc[VTIME] = 0;  // usaremos select para timeout

        tcflush(fd, TCIFLUSH);

        if (tcsetattr(fd, TCSANOW, &options) == -1) {
            std::cerr << "Error configurando puerto: " << strerror(errno) << std::endl;
            close(fd); fd = -1; return;
        }

        // IMPORTANTE: Arduino se reinicia al abrir la conexión serie en la mayoría de placas.
        // Dar un pequeño retardo para que arranque y emita sus mensajes iniciales.
        usleep(2000000); // 2 segundos

        // Limpiamos cualquier mensaje de boot que Arduino haya enviado
        tcflush(fd, TCIFLUSH);

        std::cout << "Puerto " << Port << " inicializado correctamente." << std::endl;
    }

    // Lectura robusta: espera hasta \n o hasta timeout_ms (milisegundos).
    void Serial_Com::Recepcion(char* Buffer, int Buff_Len, int timeout_ms) {
        if (fd < 0) {
            std::cerr << "Recepcion: puerto no inicializado" << std::endl;
            Buffer[0] = '\0';
            return;
        }

        int total = 0;
        Buffer[0] = '\0';

        while (total < Buff_Len - 1) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(fd, &rfds);

            struct timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            int rv = select(fd + 1, &rfds, NULL, NULL, &tv);
            if (rv == -1) {
                perror("select");
                break;
            } else if (rv == 0) {
                // timeout
                break;
            } else {
                // hay datos
                char c;
                int n = read(fd, &c, 1);
                if (n > 0) {
                    // ignorar CR
                    if (c == '\r') continue;
                    if (c == '\n') {
                        // fin de línea
                        break;
                    }
                    Buffer[total++] = c;
                } else if (n == 0) {
                    // nada leído, continuar
                    continue;
                } else {
                    // error
                    perror("read");
                    break;
                }
            }
        }

        Buffer[total] = '\0';
        if (total > 0) {
            cadena += std::string(Buffer) + "\n";
        } else {
            // vacío por timeout
        }
    }

    void Serial_Com::Transmision(std::string& Mensaje) {
        if (fd < 0) {
            std::cerr << "Transmision: puerto no inicializado" << std::endl;
            return;
        }

        std::string msgOut = Mensaje;
        
        if (msgOut.size() < 2 || msgOut.substr(msgOut.size() - 2) != "\r\n")
        msgOut += "\r\n";

        std::cout << "[Transmision] Comando enviado: " << msgOut << std::endl;
        int n = write(fd, msgOut.c_str(), msgOut.size());
        if (n < 0) {
            perror("Error al enviar el mensaje");
        } else {
            std::cout << "[Transmision] Bytes escritos: " << n << std::endl;
        }
    }

    void Serial_Com::Show_Str(std::string Mensaje) {
        std::cout << Mensaje << std::endl;
    }

    std::string Serial_Com::GetCadena() {
        return cadena;
    }

    void Serial_Com::ClearInput() {
        if (fd >= 0) {
            tcflush(fd, TCIFLUSH);
        }
    }