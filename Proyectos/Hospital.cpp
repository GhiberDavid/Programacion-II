#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>  // Para system()
#include <cstring>

using namespace std;


// CARACTERES QUE FUNCIONAN EN WINDOWS
const string ESQ_SUP_IZQ = "\xDA";      // +
const string ESQ_SUP_DER = "\xBF";      // +  
const string ESQ_INF_IZQ = "\xC0";      // +
const string ESQ_INF_DER = "\xD9";      // +
const string LINEA_HORIZ = "\xC4";      // -
const string LINEA_VERT = "\xB3";       // �
const string CRUZ_SUP = "\xC2";         // -
const string CRUZ_INF = "\xC1";         // -
const string CRUZ_IZQ = "\xC3";         // �
const string CRUZ_DER = "\xB4";         // �
const string CRUZ_CENTRO = "\xC5";      // +

//---------------------------------------------
// DECLARACIONES DE ESTRUCTURAS 
//---------------------------------------------

struct HistorialMedico {
    int idConsulta;
    char fecha[11];        // YYYY-MM-DD
    char hora[6];          // HH:MM
    char diagnostico[200];
    char tratamiento[200];
    char medicamentos[150];
    int idDoctor;
    float costo;
};

struct Cita {
    int id;
    int idPaciente;
    int idDoctor;
    char fecha[11];        // YYYY-MM-DD
    char hora[6];          // HH:MM
    char motivo[150];
    char estado[20];       // "Agendada", "Atendida", "Cancelada"
    char observaciones[200];
    bool atendida;
};

struct Paciente {
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[20];       // DEBE SER ÚNICA
    int edad;              // Validar: 0-120 años
    char sexo;             // 'M' o 'F'
    char tipoSangre[5];    // "O+", "A-", "AB+", etc.
    char telefono[15];
    char direccion[100];
    char email[50];
    
    // Array dinámico de historial médico
    HistorialMedico* historial;
    int cantidadConsultas;
    int capacidadHistorial;  // Capacidad inicial: 5
    
    // Array dinámico de IDs de citas
    int* citasAgendadas;
    int cantidadCitas;
    int capacidadCitas;      // Capacidad inicial: 5
    
    char alergias[500];      // "Alergia1, Alergia2, ..."
    char observaciones[500]; // Notas generales
    bool activo;
};

struct Doctor {
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[20];        // DEBE SER ÚNICA
    char especialidad[50];  // "Cardiología", "Pediatría", etc.
    int aniosExperiencia;   // Validar: mínimo 0
    float costoConsulta;    // Validar: mayor a 0
    char horarioAtencion[50]; // "Lun-Vie 8:00-16:00"
    char telefono[15];
    char email[50];
    
    // Array dinámico de IDs de pacientes asignados
    int* pacientesAsignados;
    int cantidadPacientes;
    int capacidadPacientes;  // Capacidad inicial: 5
    
    // Array dinámico de IDs de citas agendadas
    int* citasAgendadas;
    int cantidadCitas;
    int capacidadCitas;      // Capacidad inicial: 10
    
    bool disponible;         // Disponible para nuevos pacientes
};

struct Hospital {
    // Información básica del hospital
    char nombre[100];
    char direccion[150];
    char telefono[15];
    
    // Array dinámico de pacientes
    Paciente* pacientes;
    int cantidadPacientes;
    int capacidadPacientes;  // Capacidad inicial: 10
    
    // Array dinámico de doctores
    Doctor* doctores;
    int cantidadDoctores;
    int capacidadDoctores;   // Capacidad inicial: 10
    
    // Array dinámico de citas
    Cita* citas;
    int cantidadCitas;
    int capacidadCitas;      // Capacidad inicial: 20
    
    // Contadores automáticos de IDs (auto-increment)
    int siguienteIdPaciente;   // Inicia en 1
    int siguienteIdDoctor;     // Inicia en 1
    int siguienteIdCita;       // Inicia en 1
    int siguienteIdConsulta;   // Inicia en 1
};

//---------------------------------------------
// Funciones de interfaz 
//---------------------------------------------


void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void dibujarCuadro(int x1, int y1, int x2, int y2) {
    gotoxy(x1, y1); cout << char(201);
    gotoxy(x2, y1); cout << char(187);
    gotoxy(x1, y2); cout << char(200);
    gotoxy(x2, y2); cout << char(188);

    for (int i = x1 + 1; i < x2; i++) {
        gotoxy(i, y1); cout << char(205);
        gotoxy(i, y2); cout << char(205);
    }
    for (int i = y1 + 1; i < y2; i++) {
        gotoxy(x1, i); cout << char(186);
        gotoxy(x2, i); cout << char(186);
    }
}

string replicar(char caracter, int cantidad) {
    string resultado = "";
    for (int i = 0; i < cantidad; i++) {
        resultado += caracter;
    }
    return resultado;
}


void textoColor(int x, int y, const string& texto, int textoColor = 15, int fondoColor = 1) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, fondoColor * 16 + textoColor);
    gotoxy(x, y);
    cout << texto;
    SetConsoleTextAttribute(hConsole, 15); // Reset al color por defecto (blanco/azul)
}


string leerCampo(int x, int y, int maxlen, const string& valorInicial = "", bool soloLectura = false) {
    string texto = valorInicial;
    char c;
    
    // Limpiar área con fondo azul y mostrar valor inicial
    textoColor(x, y, replicar(' ', maxlen), 15, 1);
    
    if (soloLectura) {
        // Modo solo lectura - mostrar en color diferente (gris)
        textoColor(x, y, texto, 8, 1);
    } else {
        // Modo editable - mostrar en color normal
        textoColor(x, y, texto, 15, 1);
    }
    
    gotoxy(x + texto.size(), y);
    
    // Si es solo lectura, salir inmediatamente
    if (soloLectura) {
        return texto;
    }
    
    while (true) {
        gotoxy(x + texto.size(), y);
        c = _getch();
        
        if (c == 13) break; // Enter
        if (c == 27) {      // ESC - siempre permitido
            texto = "ESC";
            break;
        }
        if (c == 8 && !texto.empty()) { // Backspace
            texto.pop_back();
            // Borrar carácter con fondo azul
            textoColor(x + texto.size(), y, " ", 15, 1);
        }
        else if (isprint((unsigned char)c) && texto.size() < maxlen) {
            texto.push_back(c);
            // Escribir carácter con color correcto
            textoColor(x + texto.size() - 1, y, string(1, c), 15, 1);
        }
    }
    return texto;
}

string leerCampoNumerico(int x, int y, int maxlen, const string& valorInicial = "", bool soloLectura = false) {
    string texto = valorInicial;
    char c;
    
    // Limpiar área antes de leer (fondo azul, texto blanco)
    textoColor(x, y, replicar(' ', maxlen), 15, 1);
    
    if (soloLectura) {
        // Modo solo lectura - mostrar en color diferente (gris)
        textoColor(x, y, texto, 8, 1);
    } else {
        // Modo editable - mostrar en color normal
        textoColor(x, y, texto, 15, 1);
    }
    
    gotoxy(x + texto.size(), y);
    
    // Si es solo lectura, salir inmediatamente
    if (soloLectura) {
        return texto;
    }
    
    while (true) {
        gotoxy(x + texto.size(), y);
        c = _getch();
        
        if (c == 13) break; // Enter
        if (c == 27) {      // ESC - siempre permitido
            texto = "ESC";
            break;
        }
        if (c == 8 && !texto.empty()) { // Backspace
            texto.pop_back();
            // Borrar carácter con fondo azul
            textoColor(x + texto.size(), y, " ", 15, 1);
            gotoxy(x + texto.size(), y);
        }
        else if (isdigit((unsigned char)c) && texto.size() < maxlen) {
            texto.push_back(c);
            // Escribir dígito con color correcto
            textoColor(x + texto.size() - 1, y, string(1, c), 15, 1);
        }
    }
    return texto;
}

//---------------------------------------------
// FUNCIÓN AUXILIAR: COMPARAR STRINGS (Case-Insensitive)
//---------------------------------------------
bool compararStrings(const char* str1, const char* str2) {
    if (str1 == nullptr || str2 == nullptr) {
        return false;
    }
    
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (tolower(str1[i]) != tolower(str2[i])) {
            return false;
        }
        i++;
    }
    
    // Ambos deben terminar al mismo tiempo
    return (str1[i] == '\0' && str2[i] == '\0');
}

// FUNCI�N AUXILIAR: CONVERTIR STRING A MIN�SCULAS (C)
//---------------------------------------------
void stringToLower(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0' && i < 499) { // l�mite por seguridad
        dest[i] = tolower(src[i]);
        i++;
    }
    dest[i] = '\0';
}

//---------------------------------------------
// FUNCI�N AUXILIAR: BUSCAR SUBSTRING (CASE-INSENSITIVE)
//---------------------------------------------
bool contieneSubstringCaseInsensitive(const char* str, const char* substr) {
    if (str == nullptr || substr == nullptr) return false;
    
    char strLower[500];
    char substrLower[500];
    
    stringToLower(strLower, str);
    stringToLower(substrLower, substr);
    
    return strstr(strLower, substrLower) != nullptr;
}



void mostrarError(int x, int y, const char* mensaje) {

    // Limpiar Texto Anterior
    textoColor(x, y, replicar(' ', 60), 12, 1);

    // Mostrar mensaje de error
    textoColor(x, y, mensaje, 12, 1);
    _getch();
    
    // Mostrar instrucci�n para cancelar
    textoColor(x, y, "Presione ESC en cualquier campo para cancelar", 14, 1);
}




//---------------------------------------------
// FUNCIONES DE PACIENTES 
//---------------------------------------------

//---------------------------------------------
// REDIMENSIONAR ARRAY DE PACIENTES
//---------------------------------------------
void redimensionarArrayPacientes(Hospital* hospital) {
    int nuevaCapacidad = hospital->capacidadPacientes * 2;
    Paciente* nuevoArray = new Paciente[nuevaCapacidad];
    
    // Copiar pacientes existentes
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        nuevoArray[i] = hospital->pacientes[i];
    }
    
    // Liberar array antiguo y actualizar
    delete[] hospital->pacientes;
    hospital->pacientes = nuevoArray;
    hospital->capacidadPacientes = nuevaCapacidad;
    
}


Paciente* buscarPacientePorCedula(Hospital* hospital, const char* cedula) {
    if (hospital == nullptr || cedula == nullptr || hospital->cantidadPacientes == 0) {
        return nullptr;
    }
    
    // Buscar en todos los pacientes
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        Paciente* paciente = &hospital->pacientes[i];
        
        // Verificar que el paciente esté activo y comparar cédulas
        if (paciente->activo && compararStrings(paciente->cedula, cedula)) {
            return paciente; // Encontrado
        }
    }
    
    return nullptr; // No encontrado
}

//---------------------------------------------
// BUSCAR PACIENTES POR NOMBRE (PARCIAL, CASE-INSENSITIVE) - VERSI�N C
//---------------------------------------------
Paciente** buscarPacientesPorNombre(Hospital* hospital, const char* nombreBuscado, int* cantidadResultados) {
    if (hospital == nullptr || nombreBuscado == nullptr || hospital->cantidadPacientes == 0) {
        *cantidadResultados = 0;
        return nullptr;
    }
    
    // PRIMER PASO: Contar cu�ntos pacientes coinciden
    int contador = 0;
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        Paciente* paciente = &hospital->pacientes[i];
        if (paciente->activo) {
            // Crear nombre completo temporal
            char nombreCompleto[101]; // nombre(50) + espacio + apellido(50) = 101
            strcpy(nombreCompleto, paciente->nombre);
            strcat(nombreCompleto, " ");
            strcat(nombreCompleto, paciente->apellido);
            
            if (contieneSubstringCaseInsensitive(nombreCompleto, nombreBuscado)) {
                contador++;
            }
        }
    }
    
    if (contador == 0) {
        *cantidadResultados = 0;
        return nullptr;
    }
    
    // SEGUNDO PASO: Crear array din�mico y llenarlo
    Paciente** resultados = new Paciente*[contador];
    int index = 0;
    
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        Paciente* paciente = &hospital->pacientes[i];
        if (paciente->activo) {
            // Crear nombre completo temporal
            char nombreCompleto[101];
            strcpy(nombreCompleto, paciente->nombre);
            strcat(nombreCompleto, " ");
            strcat(nombreCompleto, paciente->apellido);
            
            if (contieneSubstringCaseInsensitive(nombreCompleto, nombreBuscado)) {
                resultados[index++] = paciente;
            }
        }
    }
    
    *cantidadResultados = contador;
    return resultados;
}



Paciente* buscarPacientePorId(Hospital* hospital, int id) {
    cout << "\n[FUNCION] buscarPacientePorId - A implementar...";
    _getch();
    return nullptr;
}



//---------------------------------------------
// FUNCIÓN AUXILIAR: MOSTRAR Y LIMPIAR ERROR
//---------------------------------------------
void mostrarError(int x, int y, const string& mensaje) {
    textoColor(x, y, mensaje, 12, 1);
    _getch();
    textoColor(x, y, replicar(' ', mensaje.length()), 1, 1); // Limpiar mensaje
}


Paciente* modificarPaciente(Paciente* paciente, const char* nombre, const char* apellido, 
                           const char* cedula, int edad, char sexo, const char* tipoSangre, 
                           const char* alergias, const char* telefono, const char* direccion, 
                           const char* email) {
    
    if (paciente == nullptr) {
        return nullptr;
    }
    
    // Copiar nuevos datos a la estructura del paciente
    strcpy(paciente->nombre, nombre);
    strcpy(paciente->apellido, apellido);
    strcpy(paciente->cedula, cedula);
    paciente->edad = edad;
    paciente->sexo = sexo;
    strcpy(paciente->tipoSangre, tipoSangre);
    strcpy(paciente->alergias, alergias);
    strcpy(paciente->telefono, telefono);
    strcpy(paciente->direccion, direccion);
    strcpy(paciente->email, email);
    
    return paciente;
}


//---------------------------------------------
// CREAR PACIENTE 
//---------------------------------------------
Paciente* crearPaciente(Hospital* hospital, const char* nombre, const char* apellido, 
                       const char* cedula, int edad, char sexo, const char* tipoSangre, 
                       const char* alergias, const char* telefono, const char* direccion, 
                       const char* email) {
    
    // Verificar si necesitamos redimensionar
    if (hospital->cantidadPacientes >= hospital->capacidadPacientes) {
        redimensionarArrayPacientes(hospital);
    }
    
    // Obtener referencia al nuevo paciente
    Paciente* nuevoPaciente = &hospital->pacientes[hospital->cantidadPacientes];
    
    // INICIALIZAR EL PACIENTE
    nuevoPaciente->id = hospital->siguienteIdPaciente++;
    nuevoPaciente->activo = true;
    nuevoPaciente->cantidadConsultas = 0;
    nuevoPaciente->cantidadCitas = 0;
    
    // Inicializar arrays dinámicos del paciente
    nuevoPaciente->capacidadHistorial = 5;
    nuevoPaciente->historial = new HistorialMedico[nuevoPaciente->capacidadHistorial];
    
    nuevoPaciente->capacidadCitas = 5;
    nuevoPaciente->citasAgendadas = new int[nuevoPaciente->capacidadCitas];
    
    // Inicializar strings vacíos
    strcpy(nuevoPaciente->observaciones, "");
    
    // Usar la función modificarPaciente para asignar los datos
    modificarPaciente(nuevoPaciente, nombre, apellido, cedula, edad, sexo, 
                     tipoSangre, alergias, telefono, direccion, email);
    
    // CONFIRMAR REGISTRO
    hospital->cantidadPacientes++;
    
    return nuevoPaciente;
}

//---------------------------------------------
// CAPTURAR DATOS DE PACIENTE Y CREARLO
//---------------------------------------------
void capturarDatosPaciente(Hospital* hospital, Paciente* pacienteExistente = nullptr, bool soloLectura = false) {
    // Variables locales para capturar datos
    char nombre[50], apellido[50], cedula[20], tipoSangre[5];
    char alergias[500], telefono[15], direccion[100], email[50];
    int edad;
    char sexo;
    
    // Si estamos editando, cargar los datos existentes
    bool esEdicion = (pacienteExistente != nullptr);

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 21);

    
    if (esEdicion) {
        strcpy(nombre, pacienteExistente->nombre);
        strcpy(apellido, pacienteExistente->apellido);
        strcpy(cedula, pacienteExistente->cedula);
        strcpy(tipoSangre, pacienteExistente->tipoSangre);
        strcpy(alergias, pacienteExistente->alergias);
        strcpy(telefono, pacienteExistente->telefono);
        strcpy(direccion, pacienteExistente->direccion);
        strcpy(email, pacienteExistente->email);
        edad = pacienteExistente->edad;
        sexo = pacienteExistente->sexo;
        leerCampo(25, 5, 49, nombre ,true);        
        leerCampo(25, 6, 49, apellido ,true);        
        leerCampo(25, 7, 19, cedula ,true); 
        leerCampo(25, 8, 3, to_string(edad), true);
        leerCampo(25, 9, 1, string(1, sexo), true);
        leerCampo(25, 12, 4,  tipoSangre, true);
        leerCampo(25, 13, 499, alergias, true);
        leerCampo(25, 16, 14, telefono, true);
        leerCampo(25, 17, 99, direccion, true);
        leerCampo(25, 18, 49, email, true);
		       
        
    } else {
        // Inicializar vacíos para nuevo paciente
        memset(nombre, 0, sizeof(nombre));
        memset(apellido, 0, sizeof(apellido));
        memset(cedula, 0, sizeof(cedula));
        memset(tipoSangre, 0, sizeof(tipoSangre));
        memset(alergias, 0, sizeof(alergias));
        memset(telefono, 0, sizeof(telefono));
        memset(direccion, 0, sizeof(direccion));
        memset(email, 0, sizeof(email));
        edad = 0;
        sexo = ' ';
    }
    
    
    string titulo;
    if (soloLectura) {
        titulo = "INFORMACION DEL PACIENTE";
    } else {
        titulo = esEdicion ? "MODIFICAR DATOS DEL PACIENTE" : "REGISTRO DE NUEVO PACIENTE";
    }
    textoColor(25 - titulo.length()/2, 3, titulo, 14, 1);
    
    // FORMULARIO DE CAPTURA
    textoColor(10, 4, "Datos Personales:", 14, 1);
    textoColor(10, 5, "Nombre: ", 15, 1);
    textoColor(10, 6, "Apellido: ", 15, 1);
    textoColor(10, 7, "Cedula: ", 15, 1);
    textoColor(10, 8, "Edad: ", 15, 1);
    textoColor(10, 9, "Sexo (M/F): ", 15, 1);
    
    textoColor(10, 11, "Datos Medicos:", 14, 1);
    textoColor(10, 12, "Tipo de Sangre: ", 15, 1);
    textoColor(10, 13, "Alergias: ", 15, 1);
    
    textoColor(10, 15, "Contacto:", 14, 1);
    textoColor(10, 16, "Telefono: ", 15, 1);
    textoColor(10, 17, "Direccion: ", 15, 1);
    textoColor(10, 18, "Email: ", 15, 1);
    
    if (!soloLectura) {
        textoColor(10, 20, "Presione ESC en cualquier campo para cancelar", 14, 1);
    } else {
        textoColor(10, 20, "Presione cualquier tecla para continuar...", 14, 1);
    }
    
    // CAPTURAR DATOS
    string temp;
    
    // Nombre
    do {
        temp = leerCampo(25, 5, 49, esEdicion ? nombre : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            strcpy(nombre, temp.c_str());
            break;
        }
        mostrarError(10, 20, "Error: El nombre no puede estar vacio!");
    } while (true);
    
    // Apellido
    do {
        temp = leerCampo(25, 6, 49, esEdicion ? apellido : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            strcpy(apellido, temp.c_str());
            break;
        }
        mostrarError(10, 20, "Error: El apellido no puede estar vacio!");
    } while (true);
    
    // Cédula (solo validar duplicados si es nuevo paciente)
    do {
        temp = leerCampo(25, 7, 19, esEdicion ? cedula : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            if (!esEdicion) {
                // Solo verificar duplicados para nuevos pacientes
                if (buscarPacientePorCedula(hospital, temp.c_str()) != nullptr) {
                    mostrarError(10, 20, "Error: Ya existe un paciente con esta cedula!");
                    continue;
                }
            }
            strcpy(cedula, temp.c_str());
            break;
        }
        mostrarError(10, 20, "Error: La cedula no puede estar vacia!");
    } while (true);
    
    // Edad
    do {
        string edadInicial = esEdicion ? to_string(edad) : "";
        temp = leerCampoNumerico(25, 8, 3, edadInicial, soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            edad = atoi(temp.c_str());
            if (edad >= 0 && edad <= 120) {
                break;
            }
        }
        mostrarError(10, 20, "Error: Edad debe estar entre 0 y 120 anos!");
    } while (true);
    
    // Sexo
    do {
        string sexoInicial = esEdicion ? string(1, sexo) : "";
        temp = leerCampo(25, 9, 1, sexoInicial, soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty() && (temp == "M" || temp == "m" || temp == "F" || temp == "f")) {
            sexo = toupper(temp[0]);
            break;
        }
        mostrarError(10, 20, "Error: Sexo debe ser M o F!");
    } while (true);
    
    // Tipo de Sangre
    do {
        temp = leerCampo(26, 12, 4, esEdicion ? tipoSangre : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            strcpy(tipoSangre, temp.c_str());
            break;
        }
        mostrarError(10, 20, "Error: El tipo de sangre no puede estar vacio!");
    } while (true);
    
    // Alergias
    temp = leerCampo(25, 13, 49, esEdicion ? alergias : "", soloLectura);
    if (temp == "ESC" && !soloLectura) return;
    if (!soloLectura) {
        strcpy(alergias, temp.c_str());
    }
    
    // Teléfono
    do {
        temp = leerCampo(25, 16, 14, esEdicion ? telefono : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            strcpy(telefono, temp.c_str());
            break;
        }
        mostrarError(10, 20, "Error: El telefono no puede estar vacio!");
    } while (true);
    
    // Dirección
    do {
        temp = leerCampo(25, 17, 49, esEdicion ? direccion : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            strcpy(direccion, temp.c_str());
            break;
        }
        mostrarError(10, 20, "Error: La direccion no puede estar vacia!");
    } while (true);
    
    // Email
    do {
        temp = leerCampo(25, 18, 49, esEdicion ? email : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            if (temp.find('@') != string::npos) {
                strcpy(email, temp.c_str());
                break;
            } else {
                mostrarError(10, 20, "Error: Email debe contener @!");
            }
        } else {
            mostrarError(10, 20, "Error: El email no puede estar vacio!");
        }
    } while (true);
    
    // CREAR O ACTUALIZAR PACIENTE (solo si no es solo lectura)
    if (!soloLectura) {
        if (esEdicion) {
            // Usar la función modificarPaciente
            modificarPaciente(pacienteExistente, nombre, apellido, cedula, edad, sexo,
                            tipoSangre, alergias, telefono, direccion, email);
        } else {
            // Usar la función crearPaciente
            crearPaciente(hospital, nombre, apellido, cedula, edad, sexo,
                        tipoSangre, alergias, telefono, direccion, email);
        }
    }
    
    if (soloLectura) {
     textoColor(10, 20, "Presione cualquier tecla para continuar...", 14, 1);
    _getch();
    }
    	
    // MOSTRAR CONFIRMACIÓN
    system("cls");
    dibujarCuadro(2, 3, 75, 10);
    
    string mensajeExito;
    if (soloLectura) {
        mensajeExito = "INFORMACION DEL PACIENTE";
    } else {
        mensajeExito = esEdicion ? "DATOS ACTUALIZADOS EXITOSAMENTE" : "REGISTRO EXITOSO";
    }
    textoColor(25 - mensajeExito.length()/2, 3, mensajeExito, 14, 1);
    
    if (!soloLectura) {
        textoColor(10, 5, esEdicion ? "Paciente actualizado exitosamente!" : "Paciente registrado exitosamente!", 10, 1);
    }
    
    if (!esEdicion && !soloLectura) {
        Paciente* nuevoPaciente = buscarPacientePorCedula(hospital, cedula);
        if (nuevoPaciente != nullptr) {
            textoColor(10, 6, "ID asignado: " + to_string(nuevoPaciente->id), 15, 1);
        }
    }
    
    textoColor(10, 7, "Nombre: " + string(nombre) + " " + string(apellido), 15, 1);
    textoColor(10, 8, "Cedula: " + string(cedula), 15, 1);
    textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}




//---------------------------------------------
// MOSTRAR PACIENTES EN TABLA 
//---------------------------------------------
void mostrarPacientesEnTabla(Hospital* hospital, Paciente** pacientes, int cantidad, const string& titulo) {
   
   int startX = -5; int startY = 1;

    if (pacientes == nullptr || cantidad == 0) {
        system("cls");
        system("color 1F");
        textoColor(startX + 14, startY, ESQ_SUP_IZQ + replicar(LINEA_HORIZ[0], 59) + ESQ_SUP_DER, 14, 1);
        textoColor(startX + 14, startY + 1, LINEA_VERT + "                    " + titulo + "                     " + LINEA_VERT, 14, 1);
        textoColor(startX + 14, startY + 2, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 21) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 14) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 6) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
        textoColor(startX + 14, startY + 3, LINEA_VERT + "    " + LINEA_VERT + " No se encontraron pacientes           " + LINEA_VERT + "              " + LINEA_VERT + "      " + LINEA_VERT + "          " + LINEA_VERT, 12, 1);
        textoColor(startX + 14, startY + 4, ESQ_INF_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_INF + replicar(LINEA_HORIZ[0], 21) + CRUZ_INF + replicar(LINEA_HORIZ[0], 14) + CRUZ_INF + replicar(LINEA_HORIZ[0], 6) + CRUZ_INF + replicar(LINEA_HORIZ[0], 10) + ESQ_INF_DER, 14, 1);
        textoColor(startX + 15, startY + 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    
    int y = startY;
    
    // CABECERA DE LA TABLA CON TÍTULO DINÁMICO
    textoColor(startX + 14, y++, ESQ_SUP_IZQ + replicar(LINEA_HORIZ[0], 59) + ESQ_SUP_DER, 14, 1);
    textoColor(startX + 14, y++, LINEA_VERT + "                    " + titulo + "                     " + LINEA_VERT, 14, 1);
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 21) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 14) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 6) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
    
    // ENCABEZADO DE COLUMNAS
    textoColor(startX + 16, y, "Id", 14, 1);
    textoColor(startX + 22, y, "Nombre Completo", 14, 1);
    textoColor(startX + 44, y, "Cedula", 14, 1);
    textoColor(startX + 57, y, "Edad", 14, 1);
    textoColor(startX + 64, y, "Consultas", 14, 1);

    // Líneas verticales del encabezado
    textoColor(startX + 14, y, LINEA_VERT, 14, 1);
    textoColor(startX + 19, y, LINEA_VERT, 14, 1);
    textoColor(startX + 41, y, LINEA_VERT, 14, 1);
    textoColor(startX + 56, y, LINEA_VERT, 14, 1);
    textoColor(startX + 63, y, LINEA_VERT, 14, 1);
    textoColor(startX + 74, y, LINEA_VERT, 14, 1);
    y++;
    
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 21) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 14) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 6) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
    
    // LISTA DE PACIENTES
    for (int i = 0; i < cantidad; i++) {
        Paciente* paciente = pacientes[i];
        if (paciente == nullptr || !paciente->activo) continue;
        
        // Alternar colores para mejor legibilidad
        int colorFila = (i % 2 == 0) ? 15 : 7;
        
        // Línea lateral izquierda
        textoColor(startX + 14, y, LINEA_VERT, 14, 1);
        
        // ID
        textoColor(startX + 16, y, to_string(paciente->id), colorFila, 1);
        
        // Nombre completo
        string nombreCompleto = string(paciente->nombre) + " " + string(paciente->apellido);
        if (nombreCompleto.length() > 19) {
            nombreCompleto = nombreCompleto.substr(0, 16) + "...";
        }
        textoColor(startX + 22, y, nombreCompleto, colorFila, 1);
        
        // Cédula
        textoColor(startX + 44, y, paciente->cedula, colorFila, 1);
        
        // Edad
        textoColor(startX + 59, y, to_string(paciente->edad), colorFila, 1);
        
        // Consultas
        textoColor(startX + 67, y, to_string(paciente->cantidadConsultas), colorFila, 1);
        
        // Líneas verticales
        textoColor(startX + 19, y, LINEA_VERT, 14, 1);
        textoColor(startX + 41, y, LINEA_VERT, 14, 1);
        textoColor(startX + 56, y, LINEA_VERT, 14, 1);
        textoColor(startX + 63, y, LINEA_VERT, 14, 1);
        textoColor(startX + 74, y, LINEA_VERT, 14, 1);
        
        y++;
        
        // Paginación (cada 10 pacientes)
        if (y >= (startY + 15) && i < cantidad - 1) {
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_INF + replicar(LINEA_HORIZ[0], 21) + CRUZ_INF + replicar(LINEA_HORIZ[0], 14) + CRUZ_INF + replicar(LINEA_HORIZ[0], 6) + CRUZ_INF + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
            textoColor(startX + 19, y++, "Presione cualquier tecla para ver mas...", 14, 1);
            _getch();
            
            // Nueva página
            system("cls");
            system("color 1F");
            y = startY;
            textoColor(startX + 14, y++, ESQ_SUP_IZQ + replicar(LINEA_HORIZ[0], 59) + ESQ_SUP_DER, 14, 1);
            textoColor(startX + 14, y++, LINEA_VERT + "                    " + titulo + " (Cont.)             " + LINEA_VERT, 14, 1);
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 21) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 14) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 6) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
            
            // Re-dibujar encabezado
            textoColor(startX + 16, y, "Id", 14, 1);
            textoColor(startX + 22, y, "Nombre Completo", 14, 1);
            textoColor(startX + 44, y, "Cedula", 14, 1);
            textoColor(startX + 57, y, "Edad", 14, 1);
            textoColor(startX + 64, y, "Consultas", 14, 1);
            
            textoColor(startX + 14, y, LINEA_VERT, 14, 1);
            textoColor(startX + 19, y, LINEA_VERT, 14, 1);
            textoColor(startX + 41, y, LINEA_VERT, 14, 1);
            textoColor(startX + 56, y, LINEA_VERT, 14, 1);
            textoColor(startX + 63, y, LINEA_VERT, 14, 1);
            textoColor(startX + 74, y, LINEA_VERT, 14, 1);
            y++;
            
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 21) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 14) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 6) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
        }
    }
    
    // PIE DE TABLA
    textoColor(startX + 14, y++, ESQ_INF_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_INF + replicar(LINEA_HORIZ[0], 21) + CRUZ_INF + replicar(LINEA_HORIZ[0], 14) + CRUZ_INF + replicar(LINEA_HORIZ[0], 6) + CRUZ_INF + replicar(LINEA_HORIZ[0], 10) + ESQ_INF_DER, 14, 1);
    textoColor(startX + 15, y++, "Total encontrados: " + to_string(cantidad), 14, 1);
    textoColor(startX + 15, y++, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}

//---------------------------------------------
// LISTAR PACIENTES 
//---------------------------------------------

void listarPacientes(Hospital* hospital) {
    if (hospital == nullptr) return;
    
    // Crear array de punteros a todos los pacientes activos
    Paciente** pacientesArray = new Paciente*[hospital->cantidadPacientes];
    int cantidadActivos = 0;
    
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        if (hospital->pacientes[i].activo) {
            pacientesArray[cantidadActivos++] = &hospital->pacientes[i];
        }
    }
    

    mostrarPacientesEnTabla(hospital, pacientesArray, cantidadActivos, "LISTA DE PACIENTES");
    
    // Liberar memoria
    delete[] pacientesArray;
}


//---------------------------------------------
// BUSCAR Y MOSTRAR PACIENTE POR CÉDULA
//---------------------------------------------
void buscarYMostrarPaciente(Hospital* hospital, bool soloLectura) {
    if (hospital == nullptr || hospital->cantidadPacientes == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, soloLectura ? "BUSCAR PACIENTE" : "MODIFICAR PACIENTE", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    string titulo = soloLectura ? "BUSCAR PACIENTE POR CEDULA" : "MODIFICAR DATOS DE PACIENTE";
    
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    
    textoColor(25, 3, titulo, 14, 1);
    textoColor(10, 5, "Ingrese la cedula del paciente: ", 15, 1);
    textoColor(10, 7, "Presione ESC para cancelar", 14, 1);
    
    // Leer cédula
    string cedula = leerCampo(42, 5, 19);
    
    if (cedula == "ESC") {
        return; // Usuario canceló
    }
    
    if (cedula.empty()) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, titulo, 14, 1);
        textoColor(10, 5, "Error: La cedula no puede estar vacia!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Buscar paciente por cédula
    Paciente* paciente = buscarPacientePorCedula(hospital, cedula.c_str());
    
    if (paciente == nullptr) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, titulo, 14, 1);
        textoColor(10, 5, "Error: No se encontro ningun paciente con cedula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    if (!paciente->activo) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, titulo, 14, 1);
        textoColor(10, 5, "Error: El paciente esta inactivo en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Llamar a capturarDatosPaciente con el modo correspondiente
    capturarDatosPaciente(hospital, paciente, soloLectura);
}

void buscarPacientePorCedulaVisual(Hospital* hospital) {
    buscarYMostrarPaciente(hospital, true); // true = modo solo lectura
}

//---------------------------------------------
// BUSCAR Y MOSTRAR PACIENTES POR NOMBRE (INTERFAZ DE USUARIO)
//---------------------------------------------
void buscarPacientesPorNombreVisual(Hospital* hospital) {
    if (hospital == nullptr || hospital->cantidadPacientes == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR PACIENTES POR NOMBRE", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    
    textoColor(25, 3, "BUSCAR PACIENTES POR NOMBRE", 14, 1);
    textoColor(10, 5, "Ingrese el nombre o parte del nombre a buscar: ", 15, 1);
    textoColor(10, 7, "Presione ESC para cancelar", 14, 1);
    
    // Leer nombre a buscar
    char nombreBuscado[100];
    string temp = leerCampo(55, 5, 49);
    
    if (temp == "ESC") {
        return; // Usuario canceló
    }
    
    if (temp.empty()) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR PACIENTES POR NOMBRE", 14, 1);
        textoColor(10, 5, "Error: El nombre no puede estar vacio!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    strcpy(nombreBuscado, temp.c_str());
    
    // Buscar pacientes
    int cantidadResultados = 0;
    Paciente** resultados = buscarPacientesPorNombre(hospital, nombreBuscado, &cantidadResultados);
    
    if (resultados == nullptr || cantidadResultados == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR PACIENTES POR NOMBRE", 14, 1);
        textoColor(10, 5, "No se encontraron pacientes con: '" + string(nombreBuscado) + "'", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Mostrar resultados en tabla
    string titulo = "RESULTADOS PARA: '" + string(nombreBuscado) + "'";
    mostrarPacientesEnTabla(hospital, resultados, cantidadResultados, titulo);
    
    // Liberar memoria del array de punteros (IMPORTANTE: no liberar los pacientes)
    delete[] resultados;
}



//---------------------------------------------
// ACTUALIZAR PACIENTE POR CÉDULA
//---------------------------------------------
bool actualizarPaciente(Hospital* hospital) {
    buscarYMostrarPaciente(hospital, false); // false = modo edición
    return true;
}

bool eliminarPaciente(Hospital* hospital, int id) {
    cout << "\n[FUNCION] eliminarPaciente - A implementar...";
    _getch();
    return true;
}

void mostrarHistorialMedico(Paciente* paciente) {
    cout << "\n[FUNCION] mostrarHistorialMedico - A implementar...";
    _getch();
}

//---------------------------------------------
// FUNCIONES DE DOCTORES 
//---------------------------------------------




Doctor* buscarDoctorPorCedula(Hospital* hospital, const char* cedula) {
    if (hospital == nullptr || cedula == nullptr || hospital->cantidadDoctores == 0) {
        return nullptr;
    }
    
    // Buscar en todos los doctores
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        Doctor* doctor = &hospital->doctores[i];
        
        // Comparar cédulas
        if (compararStrings(doctor->cedula, cedula)) {
            return doctor; // Encontrado
        }
    }
    
    return nullptr; // No encontrado
}

Doctor* modificarDoctor(Doctor* doctor, const char* nombre, const char* apellido,
                     const char* cedula, const char* especialidad, 
                     int aniosExperiencia, float costoConsulta,
                     const char* telefono, const char* email) {
    
    if (doctor == nullptr) {
        return nullptr;
    }
    
    // Copiar nuevos datos a la estructura del doctor
    strcpy(doctor->nombre, nombre);
    strcpy(doctor->apellido, apellido);
    strcpy(doctor->cedula, cedula);
    strcpy(doctor->especialidad, especialidad);
    doctor->aniosExperiencia = aniosExperiencia;
    doctor->costoConsulta = costoConsulta;
    strcpy(doctor->telefono, telefono);
    strcpy(doctor->email, email);
    
    return doctor;
}

//---------------------------------------------
// CREAR DOCTOR 
//---------------------------------------------
Doctor* crearDoctor(Hospital* hospital, const char* nombre, const char* apellido,
                   const char* cedula, const char* especialidad, 
                   int aniosExperiencia, float costoConsulta,
                   const char* telefono = "0261-0000000", 
                   const char* email = "doctor@hospital.com") {
    
    // Verificar si necesitamos redimensionar
    if (hospital->cantidadDoctores >= hospital->capacidadDoctores) {
        // Redimensionar array de doctores
        int nuevaCapacidad = hospital->capacidadDoctores * 2;
        Doctor* nuevoArray = new Doctor[nuevaCapacidad];
        
        for (int i = 0; i < hospital->cantidadDoctores; i++) {
            nuevoArray[i] = hospital->doctores[i];
        }
        
        delete[] hospital->doctores;
        hospital->doctores = nuevoArray;
        hospital->capacidadDoctores = nuevaCapacidad;
    }
    
    Doctor* nuevoDoctor = &hospital->doctores[hospital->cantidadDoctores];
    
    // INICIALIZAR DOCTOR
    nuevoDoctor->id = hospital->siguienteIdDoctor++;
    nuevoDoctor->disponible = true;
    nuevoDoctor->cantidadPacientes = 0;
    nuevoDoctor->cantidadCitas = 0;
    nuevoDoctor->capacidadPacientes = 5;
    nuevoDoctor->pacientesAsignados = new int[nuevoDoctor->capacidadPacientes];
    nuevoDoctor->capacidadCitas = 10;
    nuevoDoctor->citasAgendadas = new int[nuevoDoctor->capacidadCitas];
    
    // Usar la función modificarDoctor para asignar los datos
    modificarDoctor(nuevoDoctor, nombre, apellido, cedula, especialidad,
                  aniosExperiencia, costoConsulta, telefono, email);
    
    // Configuración adicional
    strcpy(nuevoDoctor->horarioAtencion, "Lun-Vie 8:00-16:00");
    
    hospital->cantidadDoctores++;
    return nuevoDoctor;
}

//---------------------------------------------
// CAPTURAR DATOS DE DOCTOR Y CREARLO
//---------------------------------------------
void capturarDatosDoctor(Hospital* hospital, Doctor* doctorExistente = nullptr, bool soloLectura = false) {
    // Variables locales para capturar datos
    char nombre[50], apellido[50], cedula[20], especialidad[50];
    char telefono[15], email[50];
    int aniosExperiencia;
    float costoConsulta;
    
    // Si estamos editando, cargar los datos existentes
    bool esEdicion = (doctorExistente != nullptr);
    
    if (esEdicion) {
        strcpy(nombre, doctorExistente->nombre);
        strcpy(apellido, doctorExistente->apellido);
        strcpy(cedula, doctorExistente->cedula);
        strcpy(especialidad, doctorExistente->especialidad);
        strcpy(telefono, doctorExistente->telefono);
        strcpy(email, doctorExistente->email);
        aniosExperiencia = doctorExistente->aniosExperiencia;
        costoConsulta = doctorExistente->costoConsulta;
    } else {
        // Inicializar vacíos para nuevo doctor
        memset(nombre, 0, sizeof(nombre));
        memset(apellido, 0, sizeof(apellido));
        memset(cedula, 0, sizeof(cedula));
        memset(especialidad, 0, sizeof(especialidad));
        memset(telefono, 0, sizeof(telefono));
        memset(email, 0, sizeof(email));
        aniosExperiencia = 0;
        costoConsulta = 0.0f;
    }
    
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 18);
    
    string titulo;
    if (soloLectura) {
        titulo = "INFORMACION DEL DOCTOR";
    } else {
        titulo = esEdicion ? "MODIFICAR DATOS DEL DOCTOR" : "REGISTRO DE NUEVO DOCTOR";
    }
    textoColor(25 - titulo.length()/2, 3, titulo, 14, 1);
    
    // FORMULARIO DE CAPTURA
    textoColor(10, 4, "Datos Personales:", 14, 1);
    textoColor(10, 5, "Nombre: ", 15, 1);
    textoColor(10, 6, "Apellido: ", 15, 1);
    textoColor(10, 7, "Cedula: ", 15, 1);
    
    textoColor(10, 9, "Datos Profesionales:", 14, 1);
    textoColor(10, 10, "Especialidad: ", 15, 1);
    textoColor(10, 11, "Años Experiencia: ", 15, 1);
    textoColor(10, 12, "Costo Consulta: ", 15, 1);
    
    textoColor(10, 14, "Contacto:", 14, 1);
    textoColor(10, 15, "Telefono: ", 15, 1);
    textoColor(10, 16, "Email: ", 15, 1);
    
    if (!soloLectura) {
        textoColor(10, 18, "Presione ESC en cualquier campo para cancelar", 14, 1);
    } else {
        textoColor(10, 18, "Presione cualquier tecla para continuar...", 14, 1);
    }
    
    // CAPTURAR DATOS
    string temp;
    
    // Nombre
    do {
        temp = leerCampo(25, 5, 49, esEdicion ? nombre : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            strcpy(nombre, temp.c_str());
            break;
        }
        mostrarError(10, 18, "Error: El nombre no puede estar vacio!");
    } while (true);
    
    // Apellido
    do {
        temp = leerCampo(25, 6, 49, esEdicion ? apellido : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            strcpy(apellido, temp.c_str());
            break;
        }
        mostrarError(10, 18, "Error: El apellido no puede estar vacio!");
    } while (true);
    
    // Cédula (solo validar duplicados si es nuevo doctor)
    do {
        temp = leerCampo(25, 7, 19, esEdicion ? cedula : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            if (!esEdicion) {
                // Verificar duplicados para nuevos doctores
                if (buscarDoctorPorCedula(hospital, temp.c_str()) != nullptr) {
                    mostrarError(10, 18, "Error: Ya existe un doctor con esta cedula!");
                    continue;
                }
            }
            strcpy(cedula, temp.c_str());
            break;
        }
        mostrarError(10, 18, "Error: La cedula no puede estar vacia!");
    } while (true);
    
    // Especialidad
    do {
        temp = leerCampo(25, 10, 49, esEdicion ? especialidad : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            strcpy(especialidad, temp.c_str());
            break;
        }
        mostrarError(10, 18, "Error: La especialidad no puede estar vacia!");
    } while (true);
    
    // Años de Experiencia
    do {
        string expInicial = esEdicion ? to_string(aniosExperiencia) : "";
        temp = leerCampoNumerico(28, 11, 2, expInicial, soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            aniosExperiencia = atoi(temp.c_str());
            if (aniosExperiencia >= 0 && aniosExperiencia <= 50) {
                break;
            }
        }
        mostrarError(10, 18, "Error: Años debe estar entre 0 y 50!");
    } while (true);
    
    // Costo Consulta
    do {
        string costoInicial = esEdicion ? to_string(costoConsulta) : "";
        temp = leerCampo(26, 12, 6, costoInicial, soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            costoConsulta = atof(temp.c_str());
            if (costoConsulta > 0) {
                break;
            }
        }
        mostrarError(10, 18, "Error: El costo debe ser mayor a 0!");
    } while (true);
    
    // Teléfono
    do {
        temp = leerCampo(25, 15, 14, esEdicion ? telefono : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            strcpy(telefono, temp.c_str());
            break;
        }
        mostrarError(10, 18, "Error: El telefono no puede estar vacio!");
    } while (true);
    
    // Email
    do {
        temp = leerCampo(25, 16, 49, esEdicion ? email : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            if (temp.find('@') != string::npos) {
                strcpy(email, temp.c_str());
                break;
            } else {
                mostrarError(10, 18, "Error: Email debe contener @!");
            }
        } else {
            mostrarError(10, 18, "Error: El email no puede estar vacio!");
        }
    } while (true);
    
    // CREAR O ACTUALIZAR DOCTOR (solo si no es solo lectura)
    Doctor* doctorResultado = nullptr;
    if (!soloLectura) {
        if (esEdicion) {
            // Usar la función modificarDoctor
            doctorResultado = modificarDoctor(doctorExistente, nombre, apellido, cedula, especialidad,
                          aniosExperiencia, costoConsulta, telefono, email);
        } else {
            // Usar la función crearDoctor
            doctorResultado = crearDoctor(hospital, nombre, apellido, cedula, especialidad,
                       aniosExperiencia, costoConsulta, telefono, email);
        }
    }
    
    // MOSTRAR CONFIRMACIÓN
    system("cls");
    dibujarCuadro(2, 3, 75, 10);
    
    string mensajeExito;
    if (soloLectura) {
        mensajeExito = "INFORMACION DEL DOCTOR";
    } else {
        mensajeExito = esEdicion ? "DATOS ACTUALIZADOS EXITOSAMENTE" : "REGISTRO EXITOSO";
    }
    textoColor(25 - mensajeExito.length()/2, 3, mensajeExito, 14, 1);
    
    if (!soloLectura) {
        textoColor(10, 5, esEdicion ? "Doctor actualizado exitosamente!" : "Doctor registrado exitosamente!", 10, 1);
    }
    
    if (!esEdicion && !soloLectura && doctorResultado != nullptr) {
        textoColor(10, 6, "ID asignado: " + to_string(doctorResultado->id), 15, 1);
    }
    
    textoColor(10, 7, "Nombre: Dr. " + string(nombre) + " " + string(apellido), 15, 1);
    textoColor(10, 8, "Especialidad: " + string(especialidad), 15, 1);
    textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}
Doctor* buscarDoctorPorId(Hospital* hospital, int id) {
    cout << "\n[FUNCION] buscarDoctorPorId - A implementar...";
    _getch();
    return nullptr;
}

//---------------------------------------------
// MOSTRAR DOCTORES EN TABLA
//---------------------------------------------
void mostrarDoctoresEnTabla(Hospital* hospital, Doctor** doctores, int cantidad, const string& titulo) {
 
    int startX = -6; int startY = 0;

    if (doctores == nullptr || cantidad == 0) {
        system("cls");
        system("color 1F");
        textoColor(startX + 14, startY, ESQ_SUP_IZQ + replicar(LINEA_HORIZ[0], 59) + ESQ_SUP_DER, 14, 1);
        textoColor(startX + 14, startY + 1, LINEA_VERT + "                    " + titulo + "                     " + LINEA_VERT, 14, 1);
        textoColor(startX + 14, startY + 2, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 21) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 14) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 6) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
        textoColor(startX + 14, startY + 3, LINEA_VERT + "    " + LINEA_VERT + " No se encontraron doctores           " + LINEA_VERT + "              " + LINEA_VERT + "      " + LINEA_VERT + "          " + LINEA_VERT, 12, 1);
        textoColor(startX + 14, startY + 4, ESQ_INF_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_INF + replicar(LINEA_HORIZ[0], 21) + CRUZ_INF + replicar(LINEA_HORIZ[0], 14) + CRUZ_INF + replicar(LINEA_HORIZ[0], 6) + CRUZ_INF + replicar(LINEA_HORIZ[0], 10) + ESQ_INF_DER, 14, 1);
        textoColor(startX + 15, startY + 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    
    int y = startY;
    
    // CABECERA DE LA TABLA CON TÍTULO DINÁMICO
    textoColor(startX + 14, y++, ESQ_SUP_IZQ + replicar(LINEA_HORIZ[0], 59) + ESQ_SUP_DER, 14, 1);
    textoColor(startX + 14, y++, LINEA_VERT + "                    " + titulo + "                      " + LINEA_VERT, 14, 1);
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 21) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 14) + CRUZ_SUP +  replicar(LINEA_HORIZ[0], 17) + CRUZ_DER, 14, 1);
    
    // ENCABEZADO DE COLUMNAS
    textoColor(startX + 16, y, "Id", 14, 1);
    textoColor(startX + 22, y, "Nombre Completo", 14, 1);
    textoColor(startX + 44, y, "Cedula", 14, 1);
    textoColor(startX + 57, y, "Especialidad", 14, 1);

    // Líneas verticales del encabezado
    textoColor(startX + 14, y, LINEA_VERT, 14, 1);
    textoColor(startX + 19, y, LINEA_VERT, 14, 1);
    textoColor(startX + 41, y, LINEA_VERT, 14, 1);
    textoColor(startX + 56, y, LINEA_VERT, 14, 1);
    textoColor(startX + 74, y, LINEA_VERT, 14, 1);
    y++;
    
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 21) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 14) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 6) +  replicar(LINEA_HORIZ[0], 11) + CRUZ_DER, 14, 1);
    
    // LISTA DE DOCTORES
    for (int i = 0; i < cantidad; i++) {
        Doctor* doctor = doctores[i];
        if (doctor == nullptr) continue;
        
        // Alternar colores para mejor legibilidad
        int colorFila = (i % 2 == 0) ? 15 : 7;
        
        // Línea lateral izquierda
        textoColor(startX + 14, y, LINEA_VERT, 14, 1);
        
        // ID
        textoColor(startX + 16, y, to_string(doctor->id), colorFila, 1);
        
        // Nombre completo
        string nombreCompleto = "Dr. " + string(doctor->nombre) + " " + string(doctor->apellido);
        if (nombreCompleto.length() > 19) {
            nombreCompleto = nombreCompleto.substr(0, 16) + "...";
        }
        textoColor(startX + 22, y, nombreCompleto, colorFila, 1);
        
        // Cédula
        textoColor(startX + 44, y, doctor->cedula, colorFila, 1);
        
        // Especialidad
        string especialidad = doctor->especialidad;
        if (especialidad.length() > 12) {
            especialidad = especialidad.substr(0, 9) + "...";
        }
        textoColor(startX + 57, y, especialidad, colorFila, 1);
        
        // Líneas verticales
        textoColor(startX + 19, y, LINEA_VERT, 14, 1);
        textoColor(startX + 41, y, LINEA_VERT, 14, 1);
        textoColor(startX + 56, y, LINEA_VERT, 14, 1);
        textoColor(startX + 74, y, LINEA_VERT, 14, 1);
        
        y++;
        
        // Paginación (cada 10 doctores)
        if (y >= (startY + 15) && i < cantidad - 1) {
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_INF + replicar(LINEA_HORIZ[0], 21) + CRUZ_INF + replicar(LINEA_HORIZ[0], 14) + CRUZ_INF + replicar(LINEA_HORIZ[0], 6) + CRUZ_INF + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
            textoColor(startX + 19, y++, "Presione cualquier tecla para ver mas...", 14, 1);
            _getch();
            
            // Nueva página
            system("cls");
            system("color 1F");
            y = startY;
            textoColor(startX + 14, y++, ESQ_SUP_IZQ + replicar(LINEA_HORIZ[0], 59) + ESQ_SUP_DER, 14, 1);
            textoColor(startX + 14, y++, LINEA_VERT + "                    " + titulo + " (Cont.)              " + LINEA_VERT, 14, 1);
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 21) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 14) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 6) + CRUZ_SUP + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
            
            // Re-dibujar encabezado
            textoColor(startX + 16, y, "Id", 14, 1);
            textoColor(startX + 22, y, "Nombre Completo", 14, 1);
            textoColor(startX + 44, y, "Cedula", 14, 1);
            textoColor(startX + 57, y, "Especialidad", 14, 1);
            
            textoColor(startX + 14, y, LINEA_VERT, 14, 1);
            textoColor(startX + 19, y, LINEA_VERT, 14, 1);
            textoColor(startX + 41, y, LINEA_VERT, 14, 1);
            textoColor(startX + 56, y, LINEA_VERT, 14, 1);
            textoColor(startX + 74, y, LINEA_VERT, 14, 1);
            y++;
            
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 21) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 14) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 6) + CRUZ_CENTRO + replicar(LINEA_HORIZ[0], 10) + CRUZ_DER, 14, 1);
        }
    }
    
    // PIE DE TABLA
    textoColor(startX + 14, y++, ESQ_INF_IZQ + replicar(LINEA_HORIZ[0], 4) + CRUZ_INF + replicar(LINEA_HORIZ[0], 21) + CRUZ_INF + replicar(LINEA_HORIZ[0], 14) + CRUZ_INF + replicar(LINEA_HORIZ[0], 6) +  replicar(LINEA_HORIZ[0], 11) + ESQ_INF_DER, 14, 1);
    textoColor(startX + 15, y++, "Total encontrados: " + to_string(cantidad), 14, 1);
    textoColor(startX + 15, y++, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}

//---------------------------------------------
// LISTAR DOCTORES 
//---------------------------------------------
void listarDoctores(Hospital* hospital) {
    if (hospital == nullptr) return;
    
    // Crear array de punteros a todos los doctores
    Doctor** doctoresArray = new Doctor*[hospital->cantidadDoctores];
    int cantidad = 0;
    
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        doctoresArray[cantidad++] = &hospital->doctores[i];
    }
    
    mostrarDoctoresEnTabla(hospital, doctoresArray, cantidad, "LISTA DE DOCTORES");
    
    // Liberar memoria
    delete[] doctoresArray;
}

bool asignarPacienteADoctor(Doctor* doctor, int idPaciente) {
    cout << "\n[FUNCION] asignarPacienteADoctor - A implementar...";
    _getch();
    return true;
}

void listarPacientesDeDoctor(Hospital* hospital, int idDoctor) {
    cout << "\n[FUNCION] listarPacientesDeDoctor - A implementar...";
    _getch();
}

bool eliminarDoctor(Hospital* hospital, int id) {
    cout << "\n[FUNCION] eliminarDoctor - A implementar...";
    _getch();
    return true;
}

//---------------------------------------------
// FUNCIONES DE CITAS 
//---------------------------------------------
Cita* agendarCita(Hospital* hospital, int idPaciente, int idDoctor,
                  const char* fecha, const char* hora, const char* motivo) {
    cout << "\n[FUNCION] agendarCita - A implementar...";
    _getch();
    return nullptr;
}

bool cancelarCita(Hospital* hospital, int idCita) {
    cout << "\n[FUNCION] cancelarCita - A implementar...";
    _getch();
    return true;
}

bool atenderCita(Hospital* hospital, int idCita, const char* diagnostico,
                 const char* tratamiento, const char* medicamentos) {
    cout << "\n[FUNCION] atenderCita - A implementar...";
    _getch();
    return true;
}

void listarCitasPendientes(Hospital* hospital) {
    cout << "\n[FUNCION] listarCitasPendientes - A implementar...";
    _getch();
}

//---------------------------------------------
// FUNCIONES DE UTILIDAD
//---------------------------------------------
Hospital* inicializarHospital(const char* nombre, const char* direccion, const char* telefono) {
    // PASO 1: Crear el hospital en memoria dinámica
    Hospital* hospital = new Hospital;
    
    // PASO 2: Copiar datos básicos del hospital
    strcpy(hospital->nombre, nombre);
    strcpy(hospital->direccion, direccion);
    strcpy(hospital->telefono, telefono);
    
    // PASO 3: Inicializar array de pacientes
    hospital->capacidadPacientes = 10;  // Según requerimientos
    hospital->pacientes = new Paciente[hospital->capacidadPacientes];
    hospital->cantidadPacientes = 0;
    
    // PASO 4: Inicializar array de doctores
    hospital->capacidadDoctores = 10;   // Según requerimientos
    hospital->doctores = new Doctor[hospital->capacidadDoctores];
    hospital->cantidadDoctores = 0;
    
    // PASO 5: Inicializar array de citas
    hospital->capacidadCitas = 20;      // Según requerimientos
    hospital->citas = new Cita[hospital->capacidadCitas];
    hospital->cantidadCitas = 0;
    
    // PASO 6: Inicializar contadores de IDs (según requerimientos)
    hospital->siguienteIdPaciente = 1;
    hospital->siguienteIdDoctor = 1;
    hospital->siguienteIdCita = 1;
    hospital->siguienteIdConsulta = 1;
    
    return hospital;
}

void destruirHospital(Hospital* hospital) {
    if (hospital == nullptr) {
        return; // No hay nada que liberar
    }
    
    // PASO 1: Liberar memoria de todos los pacientes y sus arrays dinámicos
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        Paciente* paciente = &hospital->pacientes[i];
        
        // Liberar array dinámico de historial médico del paciente
        if (paciente->historial != nullptr) {
            delete[] paciente->historial;
            paciente->historial = nullptr;
        }
        
        // Liberar array dinámico de citas agendadas del paciente
        if (paciente->citasAgendadas != nullptr) {
            delete[] paciente->citasAgendadas;
            paciente->citasAgendadas = nullptr;
        }
    }
    
    // PASO 2: Liberar array dinámico de pacientes del hospital
    if (hospital->pacientes != nullptr) {
        delete[] hospital->pacientes;
        hospital->pacientes = nullptr;
    }
    
    // PASO 3: Liberar memoria de todos los doctores y sus arrays dinámicos
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        Doctor* doctor = &hospital->doctores[i];
        
        // Liberar array dinámico de pacientes asignados del doctor
        if (doctor->pacientesAsignados != nullptr) {
            delete[] doctor->pacientesAsignados;
            doctor->pacientesAsignados = nullptr;
        }
        
        // Liberar array dinámico de citas agendadas del doctor
        if (doctor->citasAgendadas != nullptr) {
            delete[] doctor->citasAgendadas;
            doctor->citasAgendadas = nullptr;
        }
    }
    
    // PASO 4: Liberar array dinámico de doctores del hospital
    if (hospital->doctores != nullptr) {
        delete[] hospital->doctores;
        hospital->doctores = nullptr;
    }
    
    // PASO 5: Liberar array dinámico de citas del hospital
    if (hospital->citas != nullptr) {
        delete[] hospital->citas;
        hospital->citas = nullptr;
    }
    
    // PASO 6: Finalmente liberar la estructura Hospital principal
    delete hospital;
    
    cout << "\nMemoria del hospital liberada correctamente.";
}


//---------------------------------------------
// GENERAR DATOS DE PRUEBA 
//---------------------------------------------
void generarDatosPrueba(Hospital* hospital) {
    if (hospital == nullptr) return;
    
    // DATOS DE 20 DOCTORES DE PRUEBA
    Doctor* doctores[20];
    
    doctores[0] = crearDoctor(hospital, "Ghiber", "Linares", "DR-12345", "Cardiologia", 15, 80.0);
    doctores[1] = crearDoctor(hospital, "Ana", "Garcia", "DR-67890", "Pediatria", 10, 60.0);
    doctores[2] = crearDoctor(hospital, "Luis", "Martinez", "DR-11111", "Traumatologia", 8, 70.0);
    doctores[3] = crearDoctor(hospital, "Maria", "Lopez", "DR-22222", "Dermatologia", 12, 75.0);
    doctores[4] = crearDoctor(hospital, "Jose", "Gonzalez", "DR-33333", "Neurologia", 20, 90.0);
    doctores[5] = crearDoctor(hospital, "Laura", "Hernandez", "DR-44444", "Ginecologia", 14, 85.0);
    doctores[6] = crearDoctor(hospital, "Miguel", "Perez", "DR-55555", "Oftalmologia", 9, 65.0);
    doctores[7] = crearDoctor(hospital, "Elena", "Sanchez", "DR-66666", "Psiquiatria", 11, 70.0);
    doctores[8] = crearDoctor(hospital, "Roberto", "Ramirez", "DR-77777", "Cirugia General", 18, 95.0);
    doctores[9] = crearDoctor(hospital, "Carmen", "Torres", "DR-88888", "Endocrinologia", 13, 75.0);
    doctores[10] = crearDoctor(hospital, "Fernando", "Diaz", "DR-99999", "Urologia", 16, 80.0);
    doctores[11] = crearDoctor(hospital, "Patricia", "Vargas", "DR-10101", "Oncologia", 17, 85.0);
    doctores[12] = crearDoctor(hospital, "Ricardo", "Mendoza", "DR-12121", "Ortopedia", 10, 70.0);
    doctores[13] = crearDoctor(hospital, "Sofia", "Rojas", "DR-13131", "Pediatria", 8, 60.0);
    doctores[14] = crearDoctor(hospital, "Daniel", "Castro", "DR-14141", "Cardiologia", 12, 80.0);
    doctores[15] = crearDoctor(hospital, "Gabriela", "Ortega", "DR-15151", "Dermatologia", 7, 65.0);
    doctores[16] = crearDoctor(hospital, "Alejandro", "Silva", "DR-16161", "Neurologia", 15, 85.0);
    doctores[17] = crearDoctor(hospital, "Isabel", "Morales", "DR-17171", "Ginecologia", 11, 75.0);
    doctores[18] = crearDoctor(hospital, "Oscar", "Reyes", "DR-18181", "Traumatologia", 9, 70.0);
    doctores[19] = crearDoctor(hospital, "Teresa", "Flores", "DR-19191", "Psiquiatria", 14, 80.0);
    
    // DATOS DE 20 PACIENTES DE PRUEBA
    Paciente* pacientes[20];
    
    pacientes[0] = crearPaciente(hospital, "Ghiber", "Linares", "PA-12345", 18, 'M', 
                                "O+", "Penicilina, Polvo", "0414-0705771", 
                                "Av. Principal, Maracaibo", "ghiberlg@gmail.com");
    
    pacientes[1] = crearPaciente(hospital, "Maria", "Gonzalez", "V-87654321", 28, 'F',
                                "A+", "Ninguna", "0412-2222222",
                                "Sector La Trinidad, Maracaibo", "maria@email.com");
    
    pacientes[2] = crearPaciente(hospital, "Pedro", "Lopez", "V-55555555", 45, 'M',
                                "B-", "Mariscos, Acetaminofen", "0416-3333333",
                                "Urb. Santa Maria, Maracaibo", "pedro@email.com");
    
    pacientes[3] = crearPaciente(hospital, "Ana", "Martinez", "V-99999999", 22, 'F',
                                "AB+", "Polen, Acaros", "0424-4444444",
                                "Sector Los Olivos, Maracaibo", "ana@email.com");
    
    pacientes[4] = crearPaciente(hospital, "Carlos", "Herrera", "V-11111111", 50, 'M',
                                "O-", "Aspirina", "0412-5555555",
                                "Urb. La Victoria, Maracaibo", "carlos@email.com");
    
    pacientes[5] = crearPaciente(hospital, "Luisa", "Diaz", "V-22222222", 32, 'F',
                                "A-", "Yodo, Latex", "0416-6666666",
                                "Sector El Valle, Maracaibo", "luisa@email.com");
    
    pacientes[6] = crearPaciente(hospital, "Jorge", "Rojas", "V-33333333", 29, 'M',
                                "B+", "Ninguna", "0424-7777777",
                                "Av. Libertador, Maracaibo", "jorge@email.com");
    
    pacientes[7] = crearPaciente(hospital, "Elena", "Castillo", "V-44444444", 41, 'F',
                                "AB-", "Polvo, Moho", "0414-8888888",
                                "Urb. Los Cortijos, Maracaibo", "elena@email.com");
    
    pacientes[8] = crearPaciente(hospital, "Rafael", "Mendoza", "V-55556666", 38, 'M',
                                "O+", "Mariscos", "0412-9999999",
                                "Sector Santa Rosa, Maracaibo", "rafael@email.com");
    
    pacientes[9] = crearPaciente(hospital, "Carolina", "Vargas", "V-66667777", 26, 'F',
                                "A+", "Penicilina", "0416-1010101",
                                "Urb. La Lago, Maracaibo", "carolina@email.com");
    
    pacientes[10] = crearPaciente(hospital, "Andres", "Silva", "V-77778888", 55, 'M',
                                 "B-", "Polen, Acetaminofen", "0424-1212121",
                                 "Sector Los Haticos, Maracaibo", "andres@email.com");
    
    pacientes[11] = crearPaciente(hospital, "Marta", "Ortega", "V-88889999", 31, 'F',
                                 "AB+", "Ninguna", "0414-1313131",
                                 "Av. Circunvalacion, Maracaibo", "marta@email.com");
    
    pacientes[12] = crearPaciente(hospital, "Hector", "Navarro", "V-99990000", 47, 'M',
                                 "O-", "Aspirina, Codeina", "0412-1414141",
                                 "Urb. El Parral, Maracaibo", "hector@email.com");
    
    pacientes[13] = crearPaciente(hospital, "Diana", "Rios", "V-10101010", 24, 'F',
                                 "A-", "Polvo, Acaros", "0416-1515151",
                                 "Sector La Chinita, Maracaibo", "diana@email.com");
    
    pacientes[14] = crearPaciente(hospital, "Sergio", "Morales", "V-11112222", 33, 'M',
                                 "B+", "Mariscos", "0424-1616161",
                                 "Urb. Delicias, Maracaibo", "sergio@email.com");
    
    pacientes[15] = crearPaciente(hospital, "Valeria", "Castro", "V-12123333", 19, 'F',
                                 "AB-", "Penicilina, Polen", "0414-1717171",
                                 "Sector San Francisco, Maracaibo", "valeria@email.com");
    
    pacientes[16] = crearPaciente(hospital, "Roberto", "Flores", "V-13134444", 42, 'M',
                                 "O+", "Ninguna", "0412-1818181",
                                 "Urb. Raul Leoni, Maracaibo", "roberto@email.com");
    
    pacientes[17] = crearPaciente(hospital, "Natalia", "Guzman", "V-14145555", 27, 'F',
                                 "A+", "Latex, Yodo", "0416-1919191",
                                 "Sector Los Puertos, Maracaibo", "natalia@email.com");
    
    pacientes[18] = crearPaciente(hospital, "Francisco", "Salazar", "V-15156666", 39, 'M',
                                 "B-", "Acetaminofen", "0424-2020202",
                                 "Av. Bella Vista, Maracaibo", "francisco@email.com");
    
    pacientes[19] = crearPaciente(hospital, "Gabriela", "Paredes", "V-16167777", 23, 'F',
                                 "AB+", "Polvo, Moho", "0414-2121212",
                                 "Urb. La Vanega, Maracaibo", "gabriela@email.com");
    
    // CREAR ALGUNAS CITAS DE PRUEBA
    if (doctores[0] != nullptr && doctores[1] != nullptr && pacientes[0] != nullptr && 
        pacientes[1] != nullptr && pacientes[2] != nullptr) {
        
        Cita* c1 = agendarCita(hospital, pacientes[0]->id, doctores[0]->id, "2024-01-15", "09:00", "Consulta cardiologica");
        Cita* c2 = agendarCita(hospital, pacientes[1]->id, doctores[1]->id, "2024-01-16", "10:30", "Control pediatrico");
        Cita* c3 = agendarCita(hospital, pacientes[2]->id, doctores[0]->id, "2024-01-17", "11:00", "Chequeo general");
        Cita* c4 = agendarCita(hospital, pacientes[3]->id, doctores[2]->id, "2024-01-18", "08:30", "Consulta traumatologica");
        Cita* c5 = agendarCita(hospital, pacientes[4]->id, doctores[3]->id, "2024-01-19", "14:00", "Consulta dermatologica");
    }
    
    textoColor(20, 10, "Datos de prueba generados exitosamente!", 10, 1);
    textoColor(20, 11, "20 Doctores, 20 Pacientes y 5 Citas creadas", 10, 1);
    textoColor(20, 12, "Presione cualquier tecla para continuar...", 10, 1);
    _getch();
}//---------------------------------------------
// MENU PRINCIPAL
//---------------------------------------------
int mostrarMenuPrincipal(Hospital* hospital) {
    system("cls");
    system("color 1F"); // Fondo azul, texto blanco
    dibujarCuadro(5, 2, 70, 17);
    
    // Título principal en amarillo
    textoColor(23, 1, "SISTEMA DE GESTION HOSPITALARIA", 14, 1);
    
    // Nombre del hospital en verde claro
    textoColor(28, 3, hospital->nombre, 10, 1);
    
    // Opciones del menú en blanco
    textoColor(20, 5, "1. Gestion de Pacientes", 15, 1);
    textoColor(20, 6, "2. Gestion de Doctores", 15, 1);
    textoColor(20, 7, "3. Gestion de Citas", 15, 1);
    textoColor(20, 8, "ESC. Salir", 12, 1); // Salir en rojo

    // Línea separadora en gris
    textoColor(10, 11, "================================================", 7, 1);
    
    // ESTADÍSTICAS con colores
    textoColor(10, 12, "Estadisticas:", 14, 1); // Título en amarillo
    
    textoColor(10, 13, "Pacientes registrados: ", 15, 1);
    textoColor(35, 13, to_string(hospital->cantidadPacientes), 10, 1); // Número en verde
    
    textoColor(10, 14, "Doctores registrados:  ", 15, 1);
    textoColor(35, 14, to_string(hospital->cantidadDoctores), 10, 1);
    
    textoColor(10, 15, "Citas agendadas:       ", 15, 1);
    textoColor(35, 15, to_string(hospital->cantidadCitas), 10, 1);
    
    // CAPACIDAD con colores
    textoColor(40, 12, "Capacidad actual:", 14, 1);
    
    textoColor(40, 13, "Pacientes: ", 15, 1);
    textoColor(51, 13, to_string(hospital->cantidadPacientes), 10, 1); // Actual en verde
    textoColor(53, 13, "/", 15, 1);
    textoColor(54, 13, to_string(hospital->capacidadPacientes), 11, 1); // Capacidad en aguamarina
    
    textoColor(40, 14, "Doctores:  ", 15, 1);
    textoColor(51, 14, to_string(hospital->cantidadDoctores), 10, 1);
    textoColor(53, 14, "/", 15, 1);
    textoColor(54, 14, to_string(hospital->capacidadDoctores), 11, 1);
    
    textoColor(40, 15, "Citas:     ", 15, 1);
    textoColor(51, 15, to_string(hospital->cantidadCitas), 10, 1);
    textoColor(53, 15, "/", 15, 1);
    textoColor(54, 15, to_string(hospital->capacidadCitas), 11, 1);
    
    textoColor(20, 10, "Seleccione una opcion: ", 15, 1);
    
    string opcion = leerCampoNumerico(44, 10, 1);
    
    if (opcion == "ESC") {
        return 0;
    }        
    return atoi(opcion.c_str());
}

//---------------------------------------------
// MENUS DE PACIENTES
//---------------------------------------------
void menuPacientes(Hospital* hospital) {
    int opcion;
    do {
        system("cls");
        system("color 1F"); // Fondo azul, texto blanco
        dibujarCuadro(5, 2, 70, 19);
        
        // Título en amarillo
        textoColor(28, 3, "GESTION DE PACIENTES", 14, 1);
        
        // Opciones en blanco
        textoColor(20, 6, "1. Registrar nuevo paciente", 15, 1);
        textoColor(20, 7, "2. Buscar paciente por cedula", 15, 1);
        textoColor(20, 8, "3. Buscar paciente por nombre", 15, 1);
        textoColor(20, 9, "4. Ver historial medico completo", 15, 1);
        textoColor(20, 10, "5. Actualizar datos del paciente", 15, 1);
        textoColor(20, 11, "6. Listar todos los pacientes", 15, 1);
        textoColor(20, 12, "7. Eliminar paciente", 15, 1);
        textoColor(20, 13, "ESC. Volver al menu principal", 12, 1); // Rojo para salir
        
        textoColor(20, 16, "Seleccione una opcion: ", 15, 1);
        
        string opcionStr = leerCampoNumerico(44, 16, 1);
        opcion = atoi(opcionStr.c_str());
        
        if (opcionStr == "ESC") {
            return; // Salir inmediatamente
        }        
        
        switch(opcion) {
            case 1:
                capturarDatosPaciente(hospital);
                break;
            case 2:
                buscarPacientePorCedulaVisual(hospital);
                break;
            case 3:
                buscarPacientesPorNombreVisual(hospital);
                break;
            case 4:
                mostrarHistorialMedico(nullptr);
                break;
            case 5:
                actualizarPaciente(hospital);
                break;
            case 6:
                listarPacientes(hospital);
                break;
            case 7:
                eliminarPaciente(hospital, 1);
                break;
            case 0:
                return;
            default:
                textoColor(20, 18, "Opcion invalida. Presione una tecla...", 12, 1);
                _getch();
        }
    } while (opcion != 0);
}

//---------------------------------------------
// MENUS DE DOCTORES
//---------------------------------------------
void menuDoctores(Hospital* hospital) {
    int opcion;
    do {
        system("cls");
        system("color 1F"); // Fondo azul, texto blanco
        dibujarCuadro(5, 2, 70, 18);
        
        // Título en amarillo
        textoColor(28, 3, "GESTION DE DOCTORES", 14, 1);
        
        // Opciones en blanco
        textoColor(20, 6, "1. Registrar nuevo doctor", 15, 1);
        textoColor(20, 7, "2. Buscar doctor por ID", 15, 1);
        textoColor(20, 8, "3. Buscar doctores por especialidad", 15, 1);
        textoColor(20, 9, "4. Asignar paciente a doctor", 15, 1);
        textoColor(20, 10, "5. Ver pacientes asignados a doctor", 15, 1);
        textoColor(20, 11, "6. Listar todos los doctores", 15, 1);
        textoColor(20, 12, "7. Eliminar doctor", 15, 1);
        textoColor(20, 13, "ESC. Volver al menu principal", 12, 1); // Rojo para salir
        
        textoColor(20, 16, "Seleccione una opcion: ", 15, 1);
        
        string opcionStr = leerCampoNumerico(44, 16, 1);
        opcion = atoi(opcionStr.c_str());
        
        if (opcionStr == "ESC") {
            return; // Salir inmediatamente
        }            
        
        switch(opcion) {
            case 1:
                capturarDatosDoctor(hospital);
                break;
            case 2:
                buscarDoctorPorId(hospital, 1);
                break;
            case 3:
                // buscarDoctoresPorEspecialidad - pendiente
                textoColor(20, 18, "Buscar por especialidad - A implementar...", 14, 1);
                _getch();
                break;
            case 4:
                asignarPacienteADoctor(nullptr, 1);
                break;
            case 5:
                listarPacientesDeDoctor(hospital, 1);
                break;
            case 6:
                listarDoctores(hospital);
                break;
            case 7:
                eliminarDoctor(hospital, 1);
                break;
            case 0:
                return;
            default:
                textoColor(20, 18, "Opcion invalida. Presione una tecla...", 12, 1);
                _getch();
        }
    } while (opcion != 0);
}

//---------------------------------------------
// MENUS DE CITAS
//---------------------------------------------
void menuCitas(Hospital* hospital) {
    int opcion;
    do {
        system("cls");
        system("color 1F"); // Fondo azul, texto blanco
        dibujarCuadro(5, 2, 70, 17);
        
        // Título en amarillo
        textoColor(30, 3, "GESTION DE CITAS", 14, 1);
        
        // Opciones en blanco
        textoColor(20, 6, "1. Agendar nueva cita", 15, 1);
        textoColor(20, 7, "2. Cancelar cita", 15, 1);
        textoColor(20, 8, "3. Atender cita", 15, 1);
        textoColor(20, 9, "4. Ver citas de un paciente", 15, 1);
        textoColor(20, 10, "5. Ver citas de un doctor", 15, 1);
        textoColor(20, 11, "6. Ver citas de una fecha", 15, 1);
        textoColor(20, 12, "7. Ver citas pendientes", 15, 1);
        textoColor(20, 13, "ESC. Volver al menu principal", 12, 1); // Rojo para salir
        
        textoColor(20, 16, "Seleccione una opcion: ", 15, 1);
        
        string opcionStr = leerCampoNumerico(44, 16, 1);
        opcion = atoi(opcionStr.c_str());
        
        if (opcionStr == "ESC") {
            return; // Salir inmediatamente
        }            
        
        switch(opcion) {
            case 1:
                agendarCita(hospital, 1, 1, "2024-01-01", "10:00", "Consulta general");
                break;
            case 2:
                cancelarCita(hospital, 1);
                break;
            case 3:
                atenderCita(hospital, 1, "Diagnostico", "Tratamiento", "Medicamentos");
                break;
            case 4:
                // obtenerCitasDePaciente - pendiente
                textoColor(20, 18, "Citas de paciente - A implementar...", 14, 1);
                _getch();
                break;
            case 5:
                // obtenerCitasDeDoctor - pendiente
                textoColor(20, 18, "Citas de doctor - A implementar...", 14, 1);
                _getch();
                break;
            case 6:
                // obtenerCitasPorFecha - pendiente
                textoColor(20, 18, "Citas por fecha - A implementar...", 14, 1);
                _getch();
                break;
            case 7:
                listarCitasPendientes(hospital);
                break;
            case 0:
                return;
            default:
                textoColor(20, 18, "Opcion invalida. Presione una tecla...", 12, 1);
                _getch();
        }
    } while (opcion != 0);
}

//---------------------------------------------
// PROGRAMA PRINCIPAL
//---------------------------------------------
int main() {
	
	
    // Inicializar hospital con datos reales
    Hospital* hospital = inicializarHospital("HOSPITAL CENTRAL URU", 
                                           "Av. Universidad, Maracaibo", 
                                           "0261-1234567");
                                           
    generarDatosPrueba(hospital);                                           
    
    int opcion;
    do {
        opcion = mostrarMenuPrincipal(hospital); 
        
        if (opcion != 0) 
        switch(opcion) {
            case 1:
                menuPacientes(hospital);
                break;
            case 2:
                menuDoctores(hospital);
                break;
            case 3:
                menuCitas(hospital);
                break;
            default:
                textoColor(20, 20, "Opcion invalida. Presione una tecla...", 12, 1);
                _getch();
        }
    } while (opcion != 0);
    
    // Limpiar memoria
    if(hospital != nullptr) {
        destruirHospital(hospital);
    }
    
    return 0;
}
