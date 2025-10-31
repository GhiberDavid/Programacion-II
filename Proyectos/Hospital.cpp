#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>  


using namespace std;


// CARACTERES QUE FUNCIONAN EN WINDOWS
const string ES_SU_IZ = "\xDA";      // +
const string ES_SU_DE = "\xBF";      // +  
const string ES_IN_IZ = "\xC0";      // +
const string ES_IN_DE = "\xD9";      // +
const string LINEA_HO = "\xC4";      // -
const string LINEA_VE = "\xB3";       // �
const string CRUZ_SUP = "\xC2";         // -
const string CRUZ_INF = "\xC1";         // -
const string CRUZ_IZQ = "\xC3";         // �
const string CRUZ_DER = "\xB4";         // �
const string CRUZ_CEN = "\xC5";         // +

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
    char cedula[20];       // DEBE SER UNICA
    int edad;              // Validar: 0-120 años
    char sexo;             // 'M' o 'F'
    char tipoSangre[5];    // "O+", "A-", "AB+", etc.
    char telefono[15];
    char direccion[100];
    char email[50];
    
    // Array dinamico de historial medico
    HistorialMedico* historial;
    int cantidadConsultas;
    int capacidadHistorial;  // Capacidad inicial: 5
    
    // Array dinamico de IDs de citas
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
    
    // Array dinamico de IDs de pacientes asignados
    int* pacientesAsignados;
    int cantidadPacientes;
    int capacidadPacientes;  // Capacidad inicial: 5
    
    // Array dinamico de IDs de citas agendadas
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
    
    // Array dinamico de pacientes
    Paciente* pacientes;
    int cantidadPacientes;
    int capacidadPacientes;  // Capacidad inicial: 10
    
    // Array dinamico de doctores
    Doctor* doctores;
    int cantidadDoctores;
    int capacidadDoctores;   // Capacidad inicial: 10
    
    // Array dinamico de citas
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
    
    // Limpiar Area con fondo azul y mostrar valor inicial
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


//---------------------------------------------
// VALIDAR FORMATO DE FECHA (YYYY-MM-DD)
//---------------------------------------------


bool validarFecha(const char* fecha) {
    if (fecha == NULL || strlen(fecha) != 10)
        return false;

    // Verificar formato YYYY-MM-DD
    if (fecha[4] != '-' || fecha[7] != '-')
        return false;

    // Verificar que los dem�s caracteres sean d�gitos
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit((unsigned char)fecha[i]))
            return false;
    }

    // Extraer a�o, mes y d�a
    char anioStr[5], mesStr[3], diaStr[3];
    strncpy(anioStr, fecha, 4);  anioStr[4] = '\0';
    strncpy(mesStr, fecha + 5, 2); mesStr[2] = '\0';
    strncpy(diaStr, fecha + 8, 2); diaStr[2] = '\0';

    int anio = atoi(anioStr);
    int mes = atoi(mesStr);
    int dia = atoi(diaStr);

    if (anio < 2020 || anio > 2030) return false;
    if (mes < 1 || mes > 12) return false;
    if (dia < 1) return false;

    // D�as por mes
    int diasMes[] = {31,28,31,30,31,30,31,31,30,31,30,31};

    // A�o bisiesto
    bool bisiesto = (anio % 4 == 0 && (anio % 100 != 0 || anio % 400 == 0));
    if (bisiesto && mes == 2) diasMes[1] = 29;

    if (dia > diasMes[mes - 1]) return false;

    return true;
}


//---------------------------------------------
// VALIDAR FORMATO DE HORA (HH:MM)
//---------------------------------------------
bool validarHora(const char* hora) {
    if (hora == nullptr || strlen(hora) != 5) {
        return false;
    }
    
    // Verificar formato HH:MM
    if (hora[2] != ':') {
        return false;
    }
    
    // Verificar que sean d�gitos
    for (int i = 0; i < 5; i++) {
        if (i != 2 && !isdigit(hora[i])) {
            return false;
        }
    }
    
    int horas = atoi(hora);
    int minutos = atoi(hora + 3);
    
    if (horas < 0 || horas > 23) return false;
    if (minutos < 0 || minutos > 59) return false;
    
    return true;
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
        
        // Verificar que el paciente estan activo y comparar cedulas
        if (paciente->activo && compararStrings(paciente->cedula, cedula)) {
            return paciente; // Encontrado
        }
    }
    
    return nullptr; // No encontrado
}

//---------------------------------------------
// BUSCAR PACIENTES POR NOMBRE (PARCIAL, CASE-INSENSITIVE) 
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



//---------------------------------------------
// BUSCAR PACIENTE POR ID
//---------------------------------------------
Paciente* buscarPacientePorId(Hospital* hospital, int id) {
    if (hospital == nullptr || hospital->cantidadPacientes == 0) {
        return nullptr;
    }
    
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        if (hospital->pacientes[i].id == id && hospital->pacientes[i].activo) {
            return &hospital->pacientes[i];
        }
    }
    
    return nullptr; // No encontrado
}



//---------------------------------------------
// FUNCICION AUXILIAR: MOSTRAR Y LIMPIAR ERROR
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
    
    // Inicializar arrays dinAmicos del paciente
    nuevoPaciente->capacidadHistorial = 5;
    nuevoPaciente->historial = new HistorialMedico[nuevoPaciente->capacidadHistorial];
    
    nuevoPaciente->capacidadCitas = 5;
    nuevoPaciente->citasAgendadas = new int[nuevoPaciente->capacidadCitas];
    
    // Inicializar strings vacios
    strcpy(nuevoPaciente->observaciones, "");
    
    // Usar la funcionn modificarPaciente para asignar los datos
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
    
    system("color 1F");
    dibujarCuadro(2, 3, 75, 21);
    
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
    
    // cedula (solo validar duplicados si es nuevo paciente)
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
            // Usar la funcion modificarPaciente
            modificarPaciente(pacienteExistente, nombre, apellido, cedula, edad, sexo,
                            tipoSangre, alergias, telefono, direccion, email);
        } else {
            // Usar la funcion crearPaciente
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
    system("color 1F");
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
        textoColor(startX + 14, startY, ES_SU_IZ + replicar(LINEA_HO[0], 59) + ES_SU_DE, 14, 1);
        textoColor(startX + 14, startY + 1, LINEA_VE + "                    " + titulo + "                     " + LINEA_VE, 14, 1);
        textoColor(startX + 14, startY + 2, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 21) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_SUP + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
        textoColor(startX + 14, startY + 3, LINEA_VE + "    " + LINEA_VE + " No se encontraron pacientes           " + LINEA_VE + "              " + LINEA_VE + "      " + LINEA_VE + "          " + LINEA_VE, 12, 1);
        textoColor(startX + 14, startY + 4, ES_IN_IZ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 21) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_INF + replicar(LINEA_HO[0], 6) + CRUZ_INF + replicar(LINEA_HO[0], 10) + ES_IN_DE, 14, 1);
        textoColor(startX + 15, startY + 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    
    int y = startY;
    
    // CABECERA DE LA TABLA CON TÍTULO DINÁMICO
    textoColor(startX + 14, y++, ES_SU_IZ + replicar(LINEA_HO[0], 59) + ES_SU_DE, 14, 1);
    textoColor(startX + 14, y++, LINEA_VE + "                    " + titulo + "                     " + LINEA_VE, 14, 1);
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 21) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_SUP + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
    
    // ENCABEZADO DE COLUMNAS
    textoColor(startX + 16, y, "Id", 14, 1);
    textoColor(startX + 22, y, "Nombre Completo", 14, 1);
    textoColor(startX + 44, y, "Cedula", 14, 1);
    textoColor(startX + 57, y, "Edad", 14, 1);
    textoColor(startX + 64, y, "Consultas", 14, 1);

    // Líneas verticales del encabezado
    textoColor(startX + 14, y, LINEA_VE, 14, 1);
    textoColor(startX + 19, y, LINEA_VE, 14, 1);
    textoColor(startX + 41, y, LINEA_VE, 14, 1);
    textoColor(startX + 56, y, LINEA_VE, 14, 1);
    textoColor(startX + 63, y, LINEA_VE, 14, 1);
    textoColor(startX + 74, y, LINEA_VE, 14, 1);
    y++;
    
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 21) + CRUZ_CEN + replicar(LINEA_HO[0], 14) + CRUZ_CEN + replicar(LINEA_HO[0], 6) + CRUZ_CEN + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
    
    // LISTA DE PACIENTES
    for (int i = 0; i < cantidad; i++) {
        Paciente* paciente = pacientes[i];
        if (paciente == nullptr || !paciente->activo) continue;
        
        // Alternar colores para mejor legibilidad
        int colorFila = (i % 2 == 0) ? 15 : 7;
        
        // Línea lateral izquierda
        textoColor(startX + 14, y, LINEA_VE, 14, 1);
        
        // ID
        textoColor(startX + 16, y, to_string(paciente->id), colorFila, 1);
        
        // Nombre completo
        string nombreCompleto = string(paciente->nombre) + " " + string(paciente->apellido);
        if (nombreCompleto.length() > 19) {
            nombreCompleto = nombreCompleto.substr(0, 16) + "...";
        }
        textoColor(startX + 22, y, nombreCompleto, colorFila, 1);
        
        // cedula
        textoColor(startX + 44, y, paciente->cedula, colorFila, 1);
        
        // Edad
        textoColor(startX + 59, y, to_string(paciente->edad), colorFila, 1);
        
        // Consultas
        textoColor(startX + 67, y, to_string(paciente->cantidadConsultas), colorFila, 1);
        
        // Líneas verticales
        textoColor(startX + 19, y, LINEA_VE, 14, 1);
        textoColor(startX + 41, y, LINEA_VE, 14, 1);
        textoColor(startX + 56, y, LINEA_VE, 14, 1);
        textoColor(startX + 63, y, LINEA_VE, 14, 1);
        textoColor(startX + 74, y, LINEA_VE, 14, 1);
        
        y++;
        
        // Paginación (cada 10 pacientes)
        if (y >= (startY + 15) && i < cantidad - 1) {
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 21) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_INF + replicar(LINEA_HO[0], 6) + CRUZ_INF + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
            textoColor(startX + 19, y++, "Presione cualquier tecla para ver mas...", 14, 1);
            _getch();
            
            // Nueva página
            system("cls");
            system("color 1F");
            y = startY;
            textoColor(startX + 14, y++, ES_SU_IZ + replicar(LINEA_HO[0], 59) + ES_SU_DE, 14, 1);
            textoColor(startX + 14, y++, LINEA_VE + "                    " + titulo + " (Cont.)             " + LINEA_VE, 14, 1);
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 21) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_SUP + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
            
            // Re-dibujar encabezado
            textoColor(startX + 16, y, "Id", 14, 1);
            textoColor(startX + 22, y, "Nombre Completo", 14, 1);
            textoColor(startX + 44, y, "Cedula", 14, 1);
            textoColor(startX + 57, y, "Edad", 14, 1);
            textoColor(startX + 64, y, "Consultas", 14, 1);
            
            textoColor(startX + 14, y, LINEA_VE, 14, 1);
            textoColor(startX + 19, y, LINEA_VE, 14, 1);
            textoColor(startX + 41, y, LINEA_VE, 14, 1);
            textoColor(startX + 56, y, LINEA_VE, 14, 1);
            textoColor(startX + 63, y, LINEA_VE, 14, 1);
            textoColor(startX + 74, y, LINEA_VE, 14, 1);
            y++;
            
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 21) + CRUZ_CEN + replicar(LINEA_HO[0], 14) + CRUZ_CEN + replicar(LINEA_HO[0], 6) + CRUZ_CEN + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
        }
    }
    
    // PIE DE TABLA
    textoColor(startX + 14, y++, ES_IN_IZ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 21) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_INF + replicar(LINEA_HO[0], 6) + CRUZ_INF + replicar(LINEA_HO[0], 10) + ES_IN_DE, 14, 1);
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
// BUSCAR Y MOSTRAR PACIENTE POR CEDULA
//---------------------------------------------
void buscarYMostrarPaciente(Hospital* hospital, bool soloLectura) {
    if (hospital == nullptr || hospital->cantidadPacientes == 0) {
        system("cls");
        system("color 1F");
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
        return; 
    }
    
    if (cedula.empty()) {
        system("cls");
        system("color 1F");
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
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, titulo, 14, 1);
        textoColor(10, 5, "Error: No se encontro ningun paciente con cedula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    if (!paciente->activo) {
        system("cls");
        system("color 1F");
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
        system("color 1F");
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
    string temp = leerCampo(57, 5, 18);
    
    if (temp == "ESC") {
        return; 
    }
    
    if (temp.empty()) {
        system("cls");
        system("color 1F");
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
        system("color 1F");
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

//---------------------------------------------
// ELIMINAR PACIENTE POR ID
//---------------------------------------------
bool eliminarPaciente(Hospital* hospital, int id) {
    if (hospital == nullptr || hospital->cantidadPacientes == 0) {
        return false;
    }
    
    // Buscar paciente por ID
    int indicePaciente = -1;
    Paciente* pacienteAEliminar = nullptr;
    
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        if (hospital->pacientes[i].id == id && hospital->pacientes[i].activo) {
            indicePaciente = i;
            pacienteAEliminar = &hospital->pacientes[i];
            break;
        }
    }
    
    if (indicePaciente == -1 || pacienteAEliminar == nullptr) {
        return false; // Paciente no encontrado
    }
    
    // PASO 1: Liberar TODA la memoria asociada al paciente
    
    // 1a. Liberar array dinamico de historial medico
    if (pacienteAEliminar->historial != nullptr) {
        delete[] pacienteAEliminar->historial;
        pacienteAEliminar->historial = nullptr;
    }
    
    // 1b. Liberar array dinamico de citas agendadas
    if (pacienteAEliminar->citasAgendadas != nullptr) {
        delete[] pacienteAEliminar->citasAgendadas;
        pacienteAEliminar->citasAgendadas = nullptr;
    }
    
    // PASO 2: Eliminar o cancelar todas las citas asociadas
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idPaciente == id) {
            // Cambiar estado a "Cancelada"
            strcpy(hospital->citas[i].estado, "Cancelada");
            hospital->citas[i].atendida = false;
            
            // Remover de la lista de citas del doctor
            for (int j = 0; j < hospital->cantidadDoctores; j++) {
                Doctor* doctor = &hospital->doctores[j];
                for (int k = 0; k < doctor->cantidadCitas; k++) {
                    if (doctor->citasAgendadas[k] == hospital->citas[i].id) {
                        // Compactar array del doctor
                        for (int m = k; m < doctor->cantidadCitas - 1; m++) {
                            doctor->citasAgendadas[m] = doctor->citasAgendadas[m + 1];
                        }
                        doctor->cantidadCitas--;
                        break;
                    }
                }
            }
        }
    }
    
    // PASO 3: Remover el paciente de las listas de doctores asignados
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        Doctor* doctor = &hospital->doctores[i];
        
        // Buscar y remover el ID del paciente del array del doctor
        for (int j = 0; j < doctor->cantidadPacientes; j++) {
            if (doctor->pacientesAsignados[j] == id) {
                // Compactar array del doctor
                for (int k = j; k < doctor->cantidadPacientes - 1; k++) {
                    doctor->pacientesAsignados[k] = doctor->pacientesAsignados[k + 1];
                }
                doctor->cantidadPacientes--;
                break;
            }
        }
    }
    
    // PASO 4: Compactar el array de pacientes
    for (int i = indicePaciente; i < hospital->cantidadPacientes - 1; i++) {
        hospital->pacientes[i] = hospital->pacientes[i + 1];
    }
    
    hospital->cantidadPacientes--;
    
    return true;
}

//---------------------------------------------
// ELIMINAR PACIENTE POR CÉDULA (INTERFAZ DE USUARIO)
//---------------------------------------------
bool eliminarPacienteVisual(Hospital* hospital) {
    if (hospital == nullptr || hospital->cantidadPacientes == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR PACIENTE", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 10);
    
    textoColor(25, 3, "ELIMINAR PACIENTE", 14, 1);
    textoColor(10, 5, "Ingrese la cedula del paciente a eliminar: ", 15, 1);
    textoColor(10, 7, "ADVERTENCIA: Esta accion no se puede deshacer!", 12, 1);
    textoColor(10, 8, "Presione ESC para cancelar", 14, 1);
    
    // Leer cédula
    string cedula = leerCampo(52, 5, 19);
    
    if (cedula == "ESC") {
        return false; 
    }
    
    if (cedula.empty()) {
        system("cls");
        system("color 1F");        
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR PACIENTE", 14, 1);
        textoColor(10, 5, "Error: La cedula no puede estar vacia!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Buscar paciente por cedula
    Paciente* paciente = buscarPacientePorCedula(hospital, cedula.c_str());
    
    if (paciente == nullptr) {
        system("cls");
        system("color 1F");        
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR PACIENTE", 14, 1);
        textoColor(10, 5, "Error: No se encontro ningun paciente con cedula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Mostrar confirmacion
    system("cls");
    system("color 1F");    
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "CONFIRMAR ELIMINACION", 14, 1);
    textoColor(10, 5, "\xA8 Esta seguro que desea eliminar al siguiente paciente ?", 15, 1);
    textoColor(10, 6, "ID: " + to_string(paciente->id), 15, 1);
    textoColor(10, 7, "Nombre: " + string(paciente->nombre) + " " + string(paciente->apellido), 15, 1);
    textoColor(10, 8, "Cedula: " + string(paciente->cedula), 15, 1);
    textoColor(10, 9, "Esta accion eliminara tambien su historial y citas!", 12, 1);
    textoColor(10, 10, "Presione S para confirmar, cualquier otra tecla para cancelar", 14, 1);
    
    char confirmacion = _getch();
    
    if (confirmacion == 'S' || confirmacion == 's') {
        // Eliminar paciente
        bool resultado = eliminarPaciente(hospital, paciente->id);
        
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR PACIENTE", 14, 1);
        
        if (resultado) {
            textoColor(10, 5, "Paciente eliminado exitosamente!", 10, 1);
        } else {
            textoColor(10, 5, "Error: No se pudo eliminar el paciente!", 12, 1);
        }
        
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return resultado;
    }
    
    return false; 
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
    
    // Usar la funcion modificarDoctor para asignar los datos
    modificarDoctor(nuevoDoctor, nombre, apellido, cedula, especialidad,
                  aniosExperiencia, costoConsulta, telefono, email);
    
    // Configuracion adicional
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
    textoColor(10, 11, "A\xA4os Experiencia: ", 15, 1);
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
    
    // A�os de Experiencia
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
    
    // Telefono
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
            // Usar la funcion modificarDoctor
            doctorResultado = modificarDoctor(doctorExistente, nombre, apellido, cedula, especialidad,
                          aniosExperiencia, costoConsulta, telefono, email);
        } else {
            // Usar la funcion crearDoctor
            doctorResultado = crearDoctor(hospital, nombre, apellido, cedula, especialidad,
                       aniosExperiencia, costoConsulta, telefono, email);
        }
    }
    
    if (soloLectura) {
     textoColor(10, 18, "Presione cualquier tecla para continuar...", 14, 1);
    _getch();
    }    
    
    // MOSTRAR CONFIRMACION
    system("cls");
    system("color 1F");
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
//---------------------------------------------
// BUSCAR DOCTORES POR ESPECIALIDAD (CASE-INSENSITIVE)
//---------------------------------------------
Doctor** buscarDoctoresPorEspecialidad(Hospital* hospital, const char* especialidadBuscada, int* cantidadResultados) {
    if (hospital == nullptr || especialidadBuscada == nullptr || hospital->cantidadDoctores == 0) {
        *cantidadResultados = 0;
        return nullptr;
    }
    
    // PRIMER PASO: Contar cuantos doctores coinciden
    int contador = 0;
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        Doctor* doctor = &hospital->doctores[i];
        
        // Convertir a minusculas para busqueda case-insensitive
        char especialidadDoctorLower[51];
        char especialidadBuscadaLower[51];
        
        stringToLower(especialidadDoctorLower, doctor->especialidad);
        stringToLower(especialidadBuscadaLower, especialidadBuscada);
        
        if (strstr(especialidadDoctorLower, especialidadBuscadaLower) != nullptr) {
            contador++;
        }
    }
    
    if (contador == 0) {
        *cantidadResultados = 0;
        return nullptr;
    }
    
    // SEGUNDO PASO: Crear array dinamico y llenarlo
    Doctor** resultados = new Doctor*[contador];
    int index = 0;
    
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        Doctor* doctor = &hospital->doctores[i];
        
        char especialidadDoctorLower[51];
        char especialidadBuscadaLower[51];
        
        stringToLower(especialidadDoctorLower, doctor->especialidad);
        stringToLower(especialidadBuscadaLower, especialidadBuscada);
        
        if (strstr(especialidadDoctorLower, especialidadBuscadaLower) != nullptr) {
            resultados[index++] = doctor;
        }
    }
    
    *cantidadResultados = contador;
    return resultados;
}



//---------------------------------------------
// BUSCAR DOCTOR POR ID
//---------------------------------------------
Doctor* buscarDoctorPorId(Hospital* hospital, int id) {
    if (hospital == nullptr || hospital->cantidadDoctores == 0) {
        return nullptr;
    }
    
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        if (hospital->doctores[i].id == id) {
            return &hospital->doctores[i];
        }
    }
    
    return nullptr; // No encontrado
}

//---------------------------------------------
// BUSCAR Y MOSTRAR DOCTOR POR ID (INTERFAZ DE USUARIO)
//---------------------------------------------
void buscarDoctorPorIdVisual(Hospital* hospital) {
    if (hospital == nullptr || hospital->cantidadDoctores == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTOR POR ID", 14, 1);
        textoColor(10, 5, "Error: No hay doctores registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    
    textoColor(25, 3, "BUSCAR DOCTOR POR ID", 14, 1);
    textoColor(10, 5, "Ingrese el ID del doctor a buscar: ", 15, 1);
    textoColor(10, 7, "Presione ESC para cancelar", 14, 1);
    
    // Leer ID
    string temp = leerCampoNumerico(45, 5, 5);
    
    if (temp == "ESC") {
        return; 
    }
    
    if (temp.empty()) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTOR POR ID", 14, 1);
        textoColor(10, 5, "Error: El ID no puede estar vacio!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    int idBuscado = atoi(temp.c_str());
    
    if (idBuscado <= 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTOR POR ID", 14, 1);
        textoColor(10, 5, "Error: El ID debe ser un numero positivo!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Buscar doctor por ID
    Doctor* doctor = buscarDoctorPorId(hospital, idBuscado);
    
    if (doctor == nullptr) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTOR POR ID", 14, 1);
        textoColor(10, 5, "Error: No se encontro ningun doctor con ID: " + to_string(idBuscado), 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Llamar a capturarDatosDoctor en modo SOLO LECTURA
    capturarDatosDoctor(hospital, doctor, true); // true = solo lectura
}
//---------------------------------------------
// MOSTRAR DOCTORES EN TABLA
//---------------------------------------------
void mostrarDoctoresEnTabla(Hospital* hospital, Doctor** doctores, int cantidad, const string& titulo) {
 
    int startX = -6; int startY = 0;

    if (doctores == nullptr || cantidad == 0) {
        system("cls");
        system("color 1F");
        textoColor(startX + 14, startY, ES_SU_IZ + replicar(LINEA_HO[0], 59) + ES_SU_DE, 14, 1);
        textoColor(startX + 14, startY + 1, LINEA_VE + "                    " + titulo + "                     " + LINEA_VE, 14, 1);
        textoColor(startX + 14, startY + 2, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 21) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_SUP + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
        textoColor(startX + 14, startY + 3, LINEA_VE + "    " + LINEA_VE + " No se encontraron doctores           " + LINEA_VE + "              " + LINEA_VE + "      " + LINEA_VE + "          " + LINEA_VE, 12, 1);
        textoColor(startX + 14, startY + 4, ES_IN_IZ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 21) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_INF + replicar(LINEA_HO[0], 6) + CRUZ_INF + replicar(LINEA_HO[0], 10) + ES_IN_DE, 14, 1);
        textoColor(startX + 15, startY + 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    
    int y = startY;
    
    // CABECERA DE LA TABLA CON TITULO DINAMICO
    textoColor(startX + 14, y++, ES_SU_IZ + replicar(LINEA_HO[0], 59) + ES_SU_DE, 14, 1);
    textoColor(startX + 14, y++, LINEA_VE + "                    " + titulo + "                      " + LINEA_VE, 14, 1);
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 21) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_SUP +  replicar(LINEA_HO[0], 17) + CRUZ_DER, 14, 1);
    
    // ENCABEZADO DE COLUMNAS
    textoColor(startX + 16, y, "Id", 14, 1);
    textoColor(startX + 22, y, "Nombre Completo", 14, 1);
    textoColor(startX + 44, y, "Cedula", 14, 1);
    textoColor(startX + 57, y, "Especialidad", 14, 1);

    // Líneas verticales del encabezado
    textoColor(startX + 14, y, LINEA_VE, 14, 1);
    textoColor(startX + 19, y, LINEA_VE, 14, 1);
    textoColor(startX + 41, y, LINEA_VE, 14, 1);
    textoColor(startX + 56, y, LINEA_VE, 14, 1);
    textoColor(startX + 74, y, LINEA_VE, 14, 1);
    y++;
    
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 21) + CRUZ_CEN + replicar(LINEA_HO[0], 14) + CRUZ_CEN + replicar(LINEA_HO[0], 6) +  replicar(LINEA_HO[0], 11) + CRUZ_DER, 14, 1);
    
    // LISTA DE DOCTORES
    for (int i = 0; i < cantidad; i++) {
        Doctor* doctor = doctores[i];
        if (doctor == nullptr) continue;
        
        // Alternar colores para mejor legibilidad
        int colorFila = (i % 2 == 0) ? 15 : 7;
        
        // Línea lateral izquierda
        textoColor(startX + 14, y, LINEA_VE, 14, 1);
        
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
        textoColor(startX + 19, y, LINEA_VE, 14, 1);
        textoColor(startX + 41, y, LINEA_VE, 14, 1);
        textoColor(startX + 56, y, LINEA_VE, 14, 1);
        textoColor(startX + 74, y, LINEA_VE, 14, 1);
        
        y++;
        
        // Paginación (cada 10 doctores)
        if (y >= (startY + 15) && i < cantidad - 1) {
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 21) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_INF + replicar(LINEA_HO[0], 6) + CRUZ_INF + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
            textoColor(startX + 19, y++, "Presione cualquier tecla para ver mas...", 14, 1);
            _getch();
            
            // Nueva página
            system("cls");
            system("color 1F");
            y = startY;
            textoColor(startX + 14, y++, ES_SU_IZ + replicar(LINEA_HO[0], 59) + ES_SU_DE, 14, 1);
            textoColor(startX + 14, y++, LINEA_VE + "                    " + titulo + " (Cont.)              " + LINEA_VE, 14, 1);
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 21) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_SUP + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
            
            // Re-dibujar encabezado
            textoColor(startX + 16, y, "Id", 14, 1);
            textoColor(startX + 22, y, "Nombre Completo", 14, 1);
            textoColor(startX + 44, y, "Cedula", 14, 1);
            textoColor(startX + 57, y, "Especialidad", 14, 1);
            
            textoColor(startX + 14, y, LINEA_VE, 14, 1);
            textoColor(startX + 19, y, LINEA_VE, 14, 1);
            textoColor(startX + 41, y, LINEA_VE, 14, 1);
            textoColor(startX + 56, y, LINEA_VE, 14, 1);
            textoColor(startX + 74, y, LINEA_VE, 14, 1);
            y++;
            
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 21) + CRUZ_CEN + replicar(LINEA_HO[0], 14) + CRUZ_CEN + replicar(LINEA_HO[0], 6) + CRUZ_CEN + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
        }
    }
    
    // PIE DE TABLA
    textoColor(startX + 14, y++, ES_IN_IZ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 21) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_INF + replicar(LINEA_HO[0], 6) +  replicar(LINEA_HO[0], 11) + ES_IN_DE, 14, 1);
    textoColor(startX + 15, y++, "Total encontrados: " + to_string(cantidad), 14, 1);
    textoColor(startX + 15, y++, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}

//---------------------------------------------
// BUSCAR Y MOSTRAR DOCTORES POR ESPECIALIDAD (INTERFAZ DE USUARIO)
//---------------------------------------------
void buscarDoctoresPorEspecialidadVisual(Hospital* hospital) {
    if (hospital == nullptr || hospital->cantidadDoctores == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTORES POR ESPECIALIDAD", 14, 1);
        textoColor(10, 5, "Error: No hay doctores registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    
    textoColor(25, 3, "BUSCAR DOCTORES POR ESPECIALIDAD", 14, 1);
    textoColor(10, 5, "Ingrese la especialidad a buscar: ", 15, 1);
    textoColor(10, 7, "Presione ESC para cancelar", 14, 1);
    
    // Leer especialidad a buscar
    char especialidadBuscada[100];
    string temp = leerCampo(45, 5, 15);
    
    if (temp == "ESC") {
        return; 
    }
    
    if (temp.empty()) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTORES POR ESPECIALIDAD", 14, 1);
        textoColor(10, 5, "Error: La especialidad no puede estar vacia!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    strcpy(especialidadBuscada, temp.c_str());
    
    // Buscar doctores
    int cantidadResultados = 0;
    Doctor** resultados = buscarDoctoresPorEspecialidad(hospital, especialidadBuscada, &cantidadResultados);
    
    if (resultados == nullptr || cantidadResultados == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTORES POR ESPECIALIDAD", 14, 1);
        textoColor(10, 5, "No se encontraron doctores con especialidad: '" + string(especialidadBuscada) + "'", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Mostrar resultados en tabla
    string titulo = "DOCTORES DE: " + string(especialidadBuscada);
    mostrarDoctoresEnTabla(hospital, resultados, cantidadResultados, titulo);
    
    // Liberar memoria del array de punteros (IMPORTANTE: no liberar los doctores)
    delete[] resultados;
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

//---------------------------------------------
// ELIMINAR DOCTOR POR ID
//---------------------------------------------
bool eliminarDoctor(Hospital* hospital, int id) {
    if (hospital == nullptr || hospital->cantidadDoctores == 0) {
        return false;
    }
    
    // Buscar doctor por ID
    int indiceDoctor = -1;
    Doctor* doctorAEliminar = nullptr;
    
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        if (hospital->doctores[i].id == id) {
            indiceDoctor = i;
            doctorAEliminar = &hospital->doctores[i];
            break;
        }
    }
    
    if (indiceDoctor == -1 || doctorAEliminar == nullptr) {
        return false; // Doctor no encontrado
    }
    
    // PASO 1: Liberar memoria de arrays dinamicos del doctor
    
    // 1a. Liberar array dinamico de pacientes asignados
    if (doctorAEliminar->pacientesAsignados != nullptr) {
        delete[] doctorAEliminar->pacientesAsignados;
        doctorAEliminar->pacientesAsignados = nullptr;
    }
    
    // 1b. Liberar array dinamico de citas agendadas
    if (doctorAEliminar->citasAgendadas != nullptr) {
        delete[] doctorAEliminar->citasAgendadas;
        doctorAEliminar->citasAgendadas = nullptr;
    }
    
    // PASO 2: Cancelar citas asociadas
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idDoctor == id && 
            strcmp(hospital->citas[i].estado, "Agendada") == 0) {
            
            // Cambiar estado a "Cancelada"
            strcpy(hospital->citas[i].estado, "Cancelada");
            hospital->citas[i].atendida = false;
            
            // Remover de la lista de citas del paciente
            for (int j = 0; j < hospital->cantidadPacientes; j++) {
                Paciente* paciente = &hospital->pacientes[j];
                for (int k = 0; k < paciente->cantidadCitas; k++) {
                    if (paciente->citasAgendadas[k] == hospital->citas[i].id) {
                        // Compactar array del paciente
                        for (int m = k; m < paciente->cantidadCitas - 1; m++) {
                            paciente->citasAgendadas[m] = paciente->citasAgendadas[m + 1];
                        }
                        paciente->cantidadCitas--;
                        break;
                    }
                }
            }
        }
    }
    
    // PASO 3: Compactar el array de doctores
    for (int i = indiceDoctor; i < hospital->cantidadDoctores - 1; i++) {
        hospital->doctores[i] = hospital->doctores[i + 1];
    }
    
    hospital->cantidadDoctores--;
    
    return true;
}

//---------------------------------------------
// ELIMINAR DOCTOR POR CEDULA 
//---------------------------------------------
bool eliminarDoctorVisual(Hospital* hospital) {
    if (hospital == nullptr || hospital->cantidadDoctores == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No hay doctores registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 10);
    
    textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
    textoColor(10, 5, "Ingrese la cedula del doctor a eliminar: ", 15, 1);
    textoColor(10, 7, "ADVERTENCIA: Esta accion no se puede deshacer!", 12, 1);
    textoColor(10, 8, "Presione ESC para cancelar", 14, 1);
    
    // Leer cédula
    string cedula = leerCampo(52, 5, 19);
    
    if (cedula == "ESC") {
        return false; 
    }
    
    if (cedula.empty()) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        textoColor(10, 5, "Error: La cedula no puede estar vacia!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Buscar doctor por cedula
    Doctor* doctor = buscarDoctorPorCedula(hospital, cedula.c_str());
    
    if (doctor == nullptr) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No se encontro ningun doctor con cedula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Verificar si el doctor tiene pacientes asignados
    if (doctor->cantidadPacientes > 0) {
        system("cls");
        system("color 1F");        
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No se puede eliminar el doctor porque tiene pacientes asignados!", 12, 1);
        textoColor(10, 6, "Pacientes asignados: " + to_string(doctor->cantidadPacientes), 15, 1);
        textoColor(10, 7, "Debe reasignar los pacientes a otro doctor primero.", 15, 1);
        textoColor(10, 8, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Verificar si el doctor tiene citas pendientes
    bool tieneCitasPendientes = false;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].idDoctor == doctor->id && 
            strcmp(hospital->citas[i].estado, "Agendada") == 0) {
            tieneCitasPendientes = true;
            break;
        }
    }
    
    // Mostrar confirmación
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "CONFIRMAR ELIMINACION", 14, 1);
    textoColor(10, 5, "\xA8 Esta seguro que desea eliminar al siguiente doctor ?", 15, 1);
    textoColor(10, 6, "ID: " + to_string(doctor->id), 15, 1);
    textoColor(10, 7, "Nombre: Dr. " + string(doctor->nombre) + " " + string(doctor->apellido), 15, 1);
    textoColor(10, 8, "Cedula: " + string(doctor->cedula), 15, 1);
    textoColor(10, 9, "Especialidad: " + string(doctor->especialidad), 15, 1);
    
    if (tieneCitasPendientes) {
        textoColor(10, 10, "ADVERTENCIA: El doctor tiene citas pendientes que seran canceladas!", 12, 1);
    } else {
        textoColor(10, 10, "El doctor no tiene citas pendientes.", 10, 1);
    }
    
    textoColor(10, 11, "Presione S para confirmar, cualquier otra tecla para cancelar", 14, 1);
    
    char confirmacion = _getch();
    
    if (confirmacion == 'S' || confirmacion == 's') {
        // Eliminar doctor
        bool resultado = eliminarDoctor(hospital, doctor->id);
        
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        
        if (resultado) {
            textoColor(10, 5, "Doctor eliminado exitosamente!", 10, 1);
            if (tieneCitasPendientes) {
                textoColor(10, 6, "Todas las citas pendientes fueron canceladas.", 14, 1);
            }
        } else {
            textoColor(10, 5, "Error: No se pudo eliminar el doctor!", 12, 1);
        }
        
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return resultado;
    }
    
    return false; 
}

//---------------------------------------------
// FUNCIONES DE CITAS 
//---------------------------------------------
//---------------------------------------------
// VERIFICAR DISPONIBILIDAD DEL DOCTOR
//---------------------------------------------
bool verificarDisponibilidad(Hospital* hospital, int idDoctor, const char* fecha, const char* hora) {
    if (hospital == nullptr || fecha == nullptr || hora == nullptr) {
        return false;
    }
    
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        Cita* cita = &hospital->citas[i];
        
        // Verificar si el doctor tiene una cita en la misma fecha y hora
        if (cita->idDoctor == idDoctor && 
            strcmp(cita->fecha, fecha) == 0 && 
            strcmp(cita->hora, hora) == 0 &&
            strcmp(cita->estado, "Agendada") == 0) {
            return false; // No est� disponible
        }
    }
    
    return true; // Est� disponible
}

//---------------------------------------------
// REDIMENSIONAR ARRAY DE CITAS
//---------------------------------------------
void redimensionarArrayCitas(Hospital* hospital) {
    int nuevaCapacidad = hospital->capacidadCitas * 2;
    Cita* nuevoArray = new Cita[nuevaCapacidad];
    
    // Copiar citas existentes
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        nuevoArray[i] = hospital->citas[i];
    }
    
    // Liberar array antiguo y actualizar
    delete[] hospital->citas;
    hospital->citas = nuevoArray;
    hospital->capacidadCitas = nuevaCapacidad;
}

//---------------------------------------------
// AGENDAR CITA (FUNCI�N PRINCIPAL)
//---------------------------------------------
Cita* agendarCita(Hospital* hospital, int idPaciente, int idDoctor,
                  const char* fecha, const char* hora, const char* motivo) {
    
    // Validaciones b�sicas
    if (hospital == nullptr || fecha == nullptr || hora == nullptr || motivo == nullptr) {
        return nullptr;
    }
    
    // PASO 1: Verificar que el paciente exista
    Paciente* paciente = buscarPacientePorId(hospital, idPaciente);
    if (paciente == nullptr || !paciente->activo) {
        return nullptr;
    }
    
    // PASO 2: Verificar que el doctor exista
    Doctor* doctor = buscarDoctorPorId(hospital, idDoctor);
    if (doctor == nullptr) {
        return nullptr;
    }
    
    // PASO 3: Validar formato de fecha
    if (!validarFecha(fecha)) {
        return nullptr;
    }
    
    // PASO 4: Validar formato de hora
    if (!validarHora(hora)) {
        return nullptr;
    }
    
    // PASO 5: Verificar disponibilidad del doctor
    if (!verificarDisponibilidad(hospital, idDoctor, fecha, hora)) {
        return nullptr;
    }
    
    // Verificar si necesitamos redimensionar el array de citas
    if (hospital->cantidadCitas >= hospital->capacidadCitas) {
        redimensionarArrayCitas(hospital);
    }
    
    // Crear la nueva cita
    Cita* nuevaCita = &hospital->citas[hospital->cantidadCitas];
    
    // Configurar datos de la cita
    nuevaCita->id = hospital->siguienteIdCita++;
    nuevaCita->idPaciente = idPaciente;
    nuevaCita->idDoctor = idDoctor;
    strcpy(nuevaCita->fecha, fecha);
    strcpy(nuevaCita->hora, hora);
    strcpy(nuevaCita->motivo, motivo);
    strcpy(nuevaCita->estado, "Agendada");
    strcpy(nuevaCita->observaciones, "");
    nuevaCita->atendida = false;
    
    // Agregar ID de cita al array del paciente (si hay espacio)
    if (paciente->cantidadCitas >= paciente->capacidadCitas) {
        // Redimensionar array de citas del paciente
        int nuevaCapacidad = paciente->capacidadCitas * 2;
        int* nuevoArray = new int[nuevaCapacidad];
        
        for (int i = 0; i < paciente->cantidadCitas; i++) {
            nuevoArray[i] = paciente->citasAgendadas[i];
        }
        
        delete[] paciente->citasAgendadas;
        paciente->citasAgendadas = nuevoArray;
        paciente->capacidadCitas = nuevaCapacidad;
    }
    
    paciente->citasAgendadas[paciente->cantidadCitas++] = nuevaCita->id;
    
    // Agregar ID de cita al array del doctor (si hay espacio)
    if (doctor->cantidadCitas >= doctor->capacidadCitas) {
        // Redimensionar array de citas del doctor
        int nuevaCapacidad = doctor->capacidadCitas * 2;
        int* nuevoArray = new int[nuevaCapacidad];
        
        for (int i = 0; i < doctor->cantidadCitas; i++) {
            nuevoArray[i] = doctor->citasAgendadas[i];
        }
        
        delete[] doctor->citasAgendadas;
        doctor->citasAgendadas = nuevoArray;
        doctor->capacidadCitas = nuevaCapacidad;
    }
    
    doctor->citasAgendadas[doctor->cantidadCitas++] = nuevaCita->id;
    
    hospital->cantidadCitas++;
    
    return nuevaCita;
}


//---------------------------------------------
// AGENDAR CITA (INTERFAZ DE USUARIO)
//---------------------------------------------
void agendarCitaVisual(Hospital* hospital) {
    if (hospital == nullptr || hospital->cantidadPacientes == 0 || hospital->cantidadDoctores == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "AGENDAR CITA", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes o doctores registrados!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 16);
    
    textoColor(25, 3, "AGENDAR NUEVA CITA", 14, 1);
    
    // FORMULARIO DE CAPTURA
    textoColor(10, 5, "Datos de la Cita:", 14, 1);
    textoColor(10, 6, "Cedula del Paciente: ", 15, 1);
    textoColor(10, 7, "Cedula del Doctor: ", 15, 1);
    textoColor(10, 8, "Fecha (YYYY-MM-DD): ", 15, 1);
    textoColor(10, 9, "Hora (HH:MM): ", 15, 1);
    textoColor(10, 10, "Motivo: ", 15, 1);
    
    textoColor(10, 12, "Ejemplo: 2025-01-15, 09:30", 8, 1);
    textoColor(10, 14, "Presione ESC en cualquier campo para cancelar", 14, 1);
    
    // VARIABLES PARA CAPTURA
    char cedulaPaciente[20], cedulaDoctor[20], fecha[11], hora[6], motivo[150];
    memset(cedulaPaciente, 0, sizeof(cedulaPaciente));
    memset(cedulaDoctor, 0, sizeof(cedulaDoctor));
    memset(fecha, 0, sizeof(fecha));
    memset(hora, 0, sizeof(hora));
    memset(motivo, 0, sizeof(motivo));
    
    // CAPTURAR DATOS
    string temp;
    
    // Cedula del Paciente
    do {
        temp = leerCampo(32, 6, 19);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            Paciente* paciente = buscarPacientePorCedula(hospital, temp.c_str());
            if (paciente != nullptr && paciente->activo) {
                strcpy(cedulaPaciente, temp.c_str());
                break;
            } else {
                mostrarError(10, 15, "Error: No se encontro paciente activo con esa cedula!");
            }
        } else {
            mostrarError(10, 15, "Error: La cedula del paciente no puede estar vacia!");
        }
    } while (true);
    
    // Cedula del Doctor
    do {
        temp = leerCampo(32, 7, 19);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            Doctor* doctor = buscarDoctorPorCedula(hospital, temp.c_str());
            if (doctor != nullptr) {
                strcpy(cedulaDoctor, temp.c_str());
                break;
            } else {
                mostrarError(10, 15, "Error: No se encontro doctor con esa cedula!");
            }
        } else {
            mostrarError(10, 15, "Error: La cedula del doctor no puede estar vacia!");
        }
    } while (true);
    
    // Fecha
    do {
        temp = leerCampo(32, 8, 10);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            if (validarFecha(temp.c_str())) {
                strcpy(fecha, temp.c_str());
                break;
            } else {
                mostrarError(10, 15, "Error: Formato de fecha invalido! Use YYYY-MM-DD");
            }
        } else {
            mostrarError(10, 15, "Error: La fecha no puede estar vacia!");
        }
    } while (true);
    
    // Hora
    do {
        temp = leerCampo(32, 9, 5);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            if (validarHora(temp.c_str())) {
                strcpy(hora, temp.c_str());
                break;
            } else {
                mostrarError(10, 15, "Error: Formato de hora invalido! Use HH:MM");
            }
        } else {
            mostrarError(10, 15, "Error: La hora no puede estar vacia!");
        }
    } while (true);
    
    // Motivo
    do {
        temp = leerCampo(32, 10, 49);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            strcpy(motivo, temp.c_str());
            break;
        } else {
            mostrarError(10, 15, "Error: El motivo no puede estar vacio!");
        }
    } while (true);
    
    // OBTENER IDs PARA AGENDAR
    Paciente* paciente = buscarPacientePorCedula(hospital, cedulaPaciente);
    Doctor* doctor = buscarDoctorPorCedula(hospital, cedulaDoctor);
    
    if (paciente == nullptr || doctor == nullptr) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "AGENDAR CITA", 14, 1);
        textoColor(10, 5, "Error: No se pudo encontrar paciente o doctor!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // VERIFICAR DISPONIBILIDAD UNA VEZ MÁS
    if (!verificarDisponibilidad(hospital, doctor->id, fecha, hora)) {
        system("cls");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "AGENDAR CITA", 14, 1);
        textoColor(10, 5, "Error: El doctor no esta disponible en esa fecha y hora!", 12, 1);
        textoColor(10, 6, "Doctor: Dr. " + string(doctor->nombre) + " " + string(doctor->apellido), 15, 1);
        textoColor(10, 7, "Fecha: " + string(fecha) + " Hora: " + string(hora), 15, 1);
        textoColor(10, 8, "Por favor, seleccione otra fecha u horario.", 14, 1);
        textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // AGENDAR LA CITA
    Cita* nuevaCita = agendarCita(hospital, paciente->id, doctor->id, fecha, hora, motivo);
    
    // MOSTRAR RESULTADO
    system("cls");
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "AGENDAR CITA", 14, 1);
    
    if (nuevaCita != nullptr) {
        textoColor(10, 5, "¡Cita agendada exitosamente!", 10, 1);
        textoColor(10, 6, "ID de Cita: " + to_string(nuevaCita->id), 15, 1);
        textoColor(10, 7, "Paciente: " + string(paciente->nombre) + " " + string(paciente->apellido), 15, 1);
        textoColor(10, 8, "Doctor: Dr. " + string(doctor->nombre) + " " + string(doctor->apellido), 15, 1);
        textoColor(10, 9, "Fecha: " + string(fecha) + " Hora: " + string(hora), 15, 1);
        textoColor(10, 10, "Motivo: " + string(motivo), 15, 1);
    } else {
        textoColor(10, 5, "Error: No se pudo agendar la cita!", 12, 1);
        textoColor(10, 6, "Por favor, verifique los datos e intente nuevamente.", 14, 1);
    }
    
    textoColor(10, 11, "Presione cualquier tecla para continuar...", 14, 1);
    _getch();
}


//---------------------------------------------
// MOSTRAR CITAS EN TABLA
//---------------------------------------------

void mostrarCitasEnTabla(Hospital* hospital, Cita** citas, int cantidad, const string& titulo) {
    
    int startX = -6; int startY = 0;

    if (citas == nullptr || cantidad == 0) {
        system("cls");
        system("color 1F");
        textoColor(startX + 10, startY, ES_SU_IZ + replicar(LINEA_HO[0], 65) + ES_SU_DE, 14, 1);
        textoColor(startX + 10, startY + 1, LINEA_VE + "                    " + titulo + "                     " + LINEA_VE, 14, 1);
        textoColor(startX + 10, startY + 2, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 18) + CRUZ_SUP + replicar(LINEA_HO[0], 18) + CRUZ_SUP + replicar(LINEA_HO[0], 10) + CRUZ_SUP + replicar(LINEA_HO[0], 5) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_DER, 14, 1);
        textoColor(startX + 10, startY + 3, LINEA_VE + "    " + LINEA_VE + " No se encontraron citas          " + LINEA_VE + "                  " + LINEA_VE + "          " + LINEA_VE + "     " + LINEA_VE + "      " + LINEA_VE, 12, 1);
        textoColor(startX + 10, startY + 4, ES_IN_IZ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 18) + CRUZ_INF + replicar(LINEA_HO[0], 18) + CRUZ_INF + replicar(LINEA_HO[0], 10) + CRUZ_INF + replicar(LINEA_HO[0], 5) + CRUZ_INF + replicar(LINEA_HO[0], 6) + ES_IN_DE, 14, 1);
        textoColor(startX + 15, startY + 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    
    int y = startY;
    
    // CABECERA DE LA TABLA 
    textoColor(startX + 10, y++, ES_SU_IZ + replicar(LINEA_HO[0], 72) + ES_SU_DE, 14, 1);
    textoColor(startX + 10, y++, LINEA_VE + "                    " + titulo + "                     " + LINEA_VE, 14, 1);
    textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 19) + CRUZ_SUP + replicar(LINEA_HO[0], 19) + CRUZ_SUP + replicar(LINEA_HO[0], 12) + CRUZ_SUP + replicar(LINEA_HO[0], 7) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_DER, 14, 1);
    
    // ENCABEZADO DE COLUMNAS 
    textoColor(startX + 12, y, "ID", 14, 1);
    textoColor(startX + 18, y, "Paciente", 14, 1);
    textoColor(startX + 38, y, "Doctor", 14, 1);
    textoColor(startX + 57, y, "Fecha", 14, 1);
    textoColor(startX + 70, y, "Hora", 14, 1);
    textoColor(startX + 77, y, "Estado", 14, 1);

    // Lineas verticales del encabezado 
    textoColor(startX + 10, y, LINEA_VE, 14, 1);
    textoColor(startX + 15, y, LINEA_VE, 14, 1);
    textoColor(startX + 35, y, LINEA_VE, 14, 1);
    textoColor(startX + 55, y, LINEA_VE, 14, 1);
    textoColor(startX + 68, y, LINEA_VE, 14, 1);
    textoColor(startX + 76, y, LINEA_VE, 14, 1);
    textoColor(startX + 83, y, LINEA_VE, 14, 1);
    y++;
    
    textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 19) + CRUZ_CEN + replicar(LINEA_HO[0], 19) + CRUZ_CEN + replicar(LINEA_HO[0], 12) + CRUZ_CEN + replicar(LINEA_HO[0], 7) + CRUZ_CEN + replicar(LINEA_HO[0], 6) + CRUZ_DER, 14, 1);
    
    // LISTA DE CITAS
    for (int i = 0; i < cantidad; i++) {
        Cita* cita = citas[i];
        if (cita == nullptr) continue;
        
        // Obtener nombres del paciente y doctor
        Paciente* paciente = buscarPacientePorId(hospital, cita->idPaciente);
        Doctor* doctor = buscarDoctorPorId(hospital, cita->idDoctor);
        
        string nombrePaciente = "No encontrado";
        string nombreDoctor = "No encontrado";
        
        if (paciente != nullptr && paciente->activo) {
            nombrePaciente = string(paciente->nombre) + " " + string(paciente->apellido);
        }
        
        if (doctor != nullptr) {
            nombreDoctor = "Dr. " + string(doctor->nombre) + " " + string(doctor->apellido);
        }
        
        // Acortar nombres 
        if (nombrePaciente.length() > 16) {
            nombrePaciente = nombrePaciente.substr(0, 13) + "...";
        }
        if (nombreDoctor.length() > 16) {
            nombreDoctor = nombreDoctor.substr(0, 13) + "...";
        }
        
        // ABREVIAR ESTADOS
        string estadoAbreviado;
        int colorEstado = 15; // Blanco por defecto
        
        if (strcmp(cita->estado, "Agendada") == 0) {
            estadoAbreviado = "AGEN";
            colorEstado = 11; // Azul claro
        } else if (strcmp(cita->estado, "Atendida") == 0) {
            estadoAbreviado = "ATEN";
            colorEstado = 10; // Verde
        } else if (strcmp(cita->estado, "Cancelada") == 0) {
            estadoAbreviado = "CANC";
            colorEstado = 12; // Rojo
        } else {
            estadoAbreviado = cita->estado; // Por si hay otros estados
        }
        
        // Alternar colores para mejor legibilidad
        int colorFila = (i % 2 == 0) ? 15 : 7;
        
        // Línea lateral izquierda
        textoColor(startX + 10, y, LINEA_VE, 14, 1);
        
        // ID
        textoColor(startX + 12, y, to_string(cita->id), colorFila, 1);
        
        // Paciente 
        textoColor(startX + 18, y, nombrePaciente, colorFila, 1);
        
        // Doctor 
        textoColor(startX + 38, y, nombreDoctor, colorFila, 1);
        
        // Fecha 
        textoColor(startX + 57, y, cita->fecha, colorFila, 1);
        
        // Hora 
        textoColor(startX + 70, y, cita->hora, colorFila, 1);
        
        // Estado 
        textoColor(startX + 78, y, estadoAbreviado, colorEstado, 1);
        
        // Lineas verticales 
        textoColor(startX + 15, y, LINEA_VE, 14, 1);
        textoColor(startX + 35, y, LINEA_VE, 14, 1);
        textoColor(startX + 55, y, LINEA_VE, 14, 1);
        textoColor(startX + 68, y, LINEA_VE, 14, 1);
        textoColor(startX + 76, y, LINEA_VE, 14, 1);
        textoColor(startX + 83, y, LINEA_VE, 14, 1);
        
        y++;
        
        // Paginación (cada 10 citas)
        if (y >= (startY + 13) && i < cantidad - 1) {
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 19) + CRUZ_INF + replicar(LINEA_HO[0], 19) + CRUZ_INF + replicar(LINEA_HO[0], 12) + CRUZ_INF + replicar(LINEA_HO[0], 7) + CRUZ_INF + replicar(LINEA_HO[0], 6) + CRUZ_DER, 14, 1);
            textoColor(startX + 15, y++, "Presione cualquier tecla para ver mas...", 14, 1);
            _getch();
            
            // Nueva página
            system("cls");
            system("color 1F");
            y = startY;
            textoColor(startX + 10, y++, ES_SU_IZ + replicar(LINEA_HO[0], 72) + ES_SU_DE, 14, 1);
            textoColor(startX + 10, y++, LINEA_VE + "                    " + titulo + " (Cont.)             " + LINEA_VE, 14, 1);
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 19) + CRUZ_SUP + replicar(LINEA_HO[0], 19) + CRUZ_SUP + replicar(LINEA_HO[0], 12) + CRUZ_SUP + replicar(LINEA_HO[0], 7) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_DER, 14, 1);
            
            // Re-dibujar encabezado (POSICIONES AJUSTADAS)
            textoColor(startX + 12, y, "ID", 14, 1);
            textoColor(startX + 18, y, "Paciente", 14, 1);
            textoColor(startX + 38, y, "Doctor", 14, 1);
            textoColor(startX + 57, y, "Fecha", 14, 1);
            textoColor(startX + 70, y, "Hora", 14, 1);
            textoColor(startX + 77, y, "Estado", 14, 1);
            
            textoColor(startX + 10, y, LINEA_VE, 14, 1);
            textoColor(startX + 15, y, LINEA_VE, 14, 1);
            textoColor(startX + 35, y, LINEA_VE, 14, 1);
            textoColor(startX + 55, y, LINEA_VE, 14, 1);
            textoColor(startX + 68, y, LINEA_VE, 14, 1);
            textoColor(startX + 76, y, LINEA_VE, 14, 1);
            textoColor(startX + 84, y, LINEA_VE, 14, 1);
            y++;
            
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 19) + CRUZ_CEN + replicar(LINEA_HO[0], 19) + CRUZ_CEN + replicar(LINEA_HO[0], 12) + CRUZ_CEN + replicar(LINEA_HO[0], 7) + CRUZ_CEN + replicar(LINEA_HO[0], 6) + CRUZ_DER, 14, 1);
        }
    }
    
    // PIE DE TABLA 
    textoColor(startX + 10, y++, ES_IN_IZ + replicar(LINEA_HO[0], 5) + CRUZ_INF + replicar(LINEA_HO[0], 18) + CRUZ_INF + replicar(LINEA_HO[0], 18) + CRUZ_INF + replicar(LINEA_HO[0], 10) + CRUZ_INF + replicar(LINEA_HO[0], 5) + CRUZ_INF + replicar(LINEA_HO[0], 6) + ES_IN_DE, 14, 1);
    textoColor(startX + 15, y++, "Total encontradas: " + to_string(cantidad), 14, 1);
    textoColor(startX + 15, y++, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}

//---------------------------------------------
// OBTENER CITAS PENDIENTES (AGENDADAS)
//---------------------------------------------
Cita** obtenerCitasPendientes(Hospital* hospital, int* cantidadResultados) {
    if (hospital == nullptr || hospital->cantidadCitas == 0) {
        *cantidadResultados = 0;
        return nullptr;
    }
    
    // PRIMER PASO: Contar cuántas citas pendientes hay
    int contador = 0;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (strcmp(hospital->citas[i].estado, "Agendada") == 0) {
            contador++;
        }
    }
    
    if (contador == 0) {
        *cantidadResultados = 0;
        return nullptr;
    }
    
    // SEGUNDO PASO: Crear array dinamico y llenarlo
    Cita** resultados = new Cita*[contador];
    int index = 0;
    
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (strcmp(hospital->citas[i].estado, "Agendada") == 0) {
            resultados[index++] = &hospital->citas[i];
        }
    }
    
    *cantidadResultados = contador;
    return resultados;
}

//---------------------------------------------
// LISTAR CITAS PENDIENTES (INTERFAZ DE USUARIO)
//---------------------------------------------
void listarCitasPendientes(Hospital* hospital) {
    if (hospital == nullptr || hospital->cantidadCitas == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS PENDIENTES", 14, 1);
        textoColor(10, 5, "No hay citas registradas en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    // Obtener citas pendientes
    int cantidadResultados = 0;
    Cita** citasPendientes = obtenerCitasPendientes(hospital, &cantidadResultados);
    
    if (citasPendientes == nullptr || cantidadResultados == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS PENDIENTES", 14, 1);
        textoColor(10, 5, "¡Excelente! No hay citas pendientes en el sistema.", 10, 1);
        textoColor(10, 6, "Todas las citas han sido atendidas o canceladas.", 10, 1);
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Mostrar resultados en tabla
    string titulo = "CITAS PENDIENTES (" + to_string(cantidadResultados) + ")";
    mostrarCitasEnTabla(hospital, citasPendientes, cantidadResultados, titulo);
    
    // Liberar memoria del array de punteros
    delete[] citasPendientes;
}


bool cancelarCita(Hospital* hospital, int idCita) {
    cout << "\n[FUNCION] cancelarCita - A implementar...";
    _getch();
    return true;
}


//---------------------------------------------
// AGREGAR CONSULTA AL HISTORIAL MEDICO
//---------------------------------------------
void agregarConsultaAlHistorial(Paciente* paciente, HistorialMedico consulta) {
    if (paciente == nullptr) return;
    
    // Verificar si el array esta lleno
    if (paciente->cantidadConsultas >= paciente->capacidadHistorial) {
        // Redimensionar array (duplicar capacidad)
        int nuevaCapacidad = paciente->capacidadHistorial * 2;
        HistorialMedico* nuevoArray = new HistorialMedico[nuevaCapacidad];
        
        // Copiar consultas existentes
        for (int i = 0; i < paciente->cantidadConsultas; i++) {
            nuevoArray[i] = paciente->historial[i];
        }
        
        // Liberar array antiguo y actualizar
        delete[] paciente->historial;
        paciente->historial = nuevoArray;
        paciente->capacidadHistorial = nuevaCapacidad;
    }
    
    // Agregar la nueva consulta
    paciente->historial[paciente->cantidadConsultas] = consulta;
    paciente->cantidadConsultas++;
}

//---------------------------------------------
// ATENDER CITA (CREA HISTORIAL MEDICO)
//---------------------------------------------
bool atenderCita(Hospital* hospital, int idCita, const char* diagnostico,
                 const char* tratamiento, const char* medicamentos) {
    
    if (hospital == nullptr || diagnostico == nullptr || 
        tratamiento == nullptr || medicamentos == nullptr) {
        return false;
    }
    
    // PASO 1: Buscar la cita por ID
    Cita* cita = nullptr;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].id == idCita) {
            cita = &hospital->citas[i];
            break;
        }
    }
    
    if (cita == nullptr) {
        return false;
    }
    
    // PASO 2: Verificar que esté en estado "Agendada"
    if (strcmp(cita->estado, "Agendada") != 0) {
        return false; // Ya fue atendida o cancelada
    }
    
    // PASO 3: Cambiar estado a "Atendida"
    strcpy(cita->estado, "Atendida");
    cita->atendida = true;
    
    // Obtener paciente y doctor
    Paciente* paciente = buscarPacientePorId(hospital, cita->idPaciente);
    Doctor* doctor = buscarDoctorPorId(hospital, cita->idDoctor);
    
    if (paciente == nullptr || doctor == nullptr) {
        return false;
    }
    
    // PASO 4: Crear estructura HistorialMedico
    HistorialMedico nuevaConsulta;
    nuevaConsulta.idConsulta = hospital->siguienteIdConsulta++;
    strcpy(nuevaConsulta.fecha, cita->fecha);
    strcpy(nuevaConsulta.hora, cita->hora);
    strcpy(nuevaConsulta.diagnostico, diagnostico);
    strcpy(nuevaConsulta.tratamiento, tratamiento);
    strcpy(nuevaConsulta.medicamentos, medicamentos);
    nuevaConsulta.idDoctor = cita->idDoctor;
    nuevaConsulta.costo = doctor->costoConsulta;
    
    // PASO 5: Agregar al historial del paciente
    agregarConsultaAlHistorial(paciente, nuevaConsulta);
    
    return true;
}


//---------------------------------------------
// ATENDER CITA (INTERFAZ DE USUARIO)
//---------------------------------------------
void atenderCitaVisual(Hospital* hospital) {
    if (hospital == nullptr || hospital->cantidadCitas == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        textoColor(10, 5, "Error: No hay citas registradas en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 10);
    
    textoColor(25, 3, "ATENDER CITA", 14, 1);
    textoColor(10, 5, "Ingrese el ID de la cita a atender: ", 15, 1);
    textoColor(10, 7, "Nota: Solo se pueden atender citas en estado 'AGEN'", 14, 1);
    textoColor(10, 9, "Presione ESC para cancelar", 14, 1);
    
    // Leer ID de la cita
    string temp = leerCampoNumerico(45, 5, 5);
    
    if (temp == "ESC") {
        return; 
    }
    
    if (temp.empty()) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        textoColor(10, 5, "Error: El ID no puede estar vacio!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    int idCita = atoi(temp.c_str());
    
    // Buscar la cita
    Cita* cita = nullptr;
    for (int i = 0; i < hospital->cantidadCitas; i++) {
        if (hospital->citas[i].id == idCita) {
            cita = &hospital->citas[i];
            break;
        }
    }
    
    if (cita == nullptr) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        textoColor(10, 5, "Error: No se encontro la cita con ID: " + to_string(idCita), 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Verificar que la cita esté agendada
    if (strcmp(cita->estado, "Agendada") != 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        textoColor(10, 5, "Error: La cita no esta en estado 'Agendada'!", 12, 1);
        textoColor(10, 6, "ID Cita: " + to_string(cita->id), 15, 1);
        textoColor(10, 7, "Estado actual: " + string(cita->estado), 15, 1);
        textoColor(10, 8, "Solo se pueden atender citas en estado 'Agendada'", 14, 1);
        textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Obtener información del paciente y doctor
    Paciente* paciente = buscarPacientePorId(hospital, cita->idPaciente);
    Doctor* doctor = buscarDoctorPorId(hospital, cita->idDoctor);
    
    if (paciente == nullptr || doctor == nullptr) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        textoColor(10, 5, "Error: No se pudo encontrar paciente o doctor!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // MOSTRAR INFORMACIÓN DE LA CITA
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 18);
    
    textoColor(25, 3, "REGISTRAR ATENCIÓN MÉDICA", 14, 1);
    textoColor(10, 5, "Informacion de la Cita:", 14, 1);
    textoColor(10, 6, "Paciente: " + string(paciente->nombre) + " " + string(paciente->apellido), 15, 1);
    textoColor(10, 7, "Doctor: Dr. " + string(doctor->nombre) + " " + string(doctor->apellido), 15, 1);
    textoColor(10, 8, "Fecha: " + string(cita->fecha) + " Hora: " + string(cita->hora), 15, 1);
    textoColor(10, 9, "Motivo: " + string(cita->motivo), 15, 1);
    textoColor(10, 10, "Costo de consulta: $" + to_string(doctor->costoConsulta), 15, 1);
    
    textoColor(10, 12, "Datos Médicos de la Consulta:", 14, 1);
    textoColor(10, 13, "Diagnostico: ", 15, 1);
    textoColor(10, 14, "Tratamiento: ", 15, 1);
    textoColor(10, 15, "Medicamentos: ", 15, 1);
    textoColor(10, 17, "Presione ESC en cualquier campo para cancelar", 14, 1);
    
    // CAPTURAR DATOS MEDICOS
    char diagnostico[200], tratamiento[200], medicamentos[150];
    memset(diagnostico, 0, sizeof(diagnostico));
    memset(tratamiento, 0, sizeof(tratamiento));
    memset(medicamentos, 0, sizeof(medicamentos));
    
    string tempStr;
    
    // diagnostico
    do {
        tempStr = leerCampo(25, 13, 49);
        if (tempStr == "ESC") return;
        
        if (!tempStr.empty()) {
            strcpy(diagnostico, tempStr.c_str());
            break;
        } else {
            mostrarError(10, 18, "Error: El diagnostico no puede estar vacio!");
        }
    } while (true);
    
    // Tratamiento
    do {
        tempStr = leerCampo(25, 14, 49);
        if (tempStr == "ESC") return;
        
        if (!tempStr.empty()) {
            strcpy(tratamiento, tempStr.c_str());
            break;
        } else {
            mostrarError(10, 18, "Error: El tratamiento no puede estar vacio!");
        }
    } while (true);
    
    // Medicamentos
    tempStr = leerCampo(25, 15, 49, "", false);
    if (tempStr == "ESC") return;
    strcpy(medicamentos, tempStr.c_str());
    
    // CONFIRMAR ATENCIÓN
    system("cls");
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "CONFIRMAR ATENCION", 14, 1);
    textoColor(10, 5, "¿Confirmar atencion de la cita?", 15, 1);
    textoColor(10, 6, "Paciente: " + string(paciente->nombre) + " " + string(paciente->apellido), 15, 1);
    textoColor(10, 7, "Diagnostico: " + string(diagnostico), 15, 1);
    textoColor(10, 8, "Tratamiento: " + string(tratamiento), 15, 1);
    textoColor(10, 9, "Costo: $" + to_string(doctor->costoConsulta), 15, 1);
    textoColor(10, 10, "Presione S para confirmar, cualquier otra tecla para cancelar", 14, 1);
    
    char confirmacion = _getch();
    
    if (confirmacion == 'S' || confirmacion == 's') {
        // Atender la cita (esto crea el historial medico automaticamente)
        bool resultado = atenderCita(hospital, idCita, diagnostico, tratamiento, medicamentos);
        
        system("cls");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        
        if (resultado) {
            textoColor(10, 5, "¡Cita atendida exitosamente!", 10, 1);
            textoColor(10, 6, "Se ha generado el historial medico del paciente.", 10, 1);
            textoColor(10, 7, "ID de Consulta: " + to_string(hospital->siguienteIdConsulta - 1), 15, 1);
            textoColor(10, 8, "Estado actualizado: ATEN", 15, 1);
        } else {
            textoColor(10, 5, "Error: No se pudo atender la cita!", 12, 1);
        }
        
        textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
    }
}


//---------------------------------------------
// MOSTRAR HISTORIAL MEDICO EN TABLA 
//---------------------------------------------
void mostrarHistorialMedicoEnTabla(Hospital* hospital, Paciente* paciente, const string& titulo) {
    
    int startX = -6; int startY = 0;

    if (paciente == nullptr || paciente->cantidadConsultas == 0) {
        system("cls");
        system("color 1F");
        textoColor(startX + 10, startY, ES_SU_IZ + replicar(LINEA_HO[0], 58) + ES_SU_DE, 14, 1);
        textoColor(startX + 10, startY + 1, LINEA_VE + "              " + titulo + "               " + LINEA_VE, 14, 1);
        textoColor(startX + 10, startY + 2, CRUZ_IZQ + replicar(LINEA_HO[0], 12) + CRUZ_SUP + replicar(LINEA_HO[0], 15) + CRUZ_SUP + replicar(LINEA_HO[0], 15) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_DER, 14, 1);
        textoColor(startX + 10, startY + 3, LINEA_VE + "            " + LINEA_VE + "               " + LINEA_VE + "               " + LINEA_VE + "              " + LINEA_VE, 12, 1);
        textoColor(startX + 22, startY + 3, " No tiene consultas en su historial ", 12, 1);
        textoColor(startX + 10, startY + 4, ES_IN_IZ + replicar(LINEA_HO[0], 12) + CRUZ_INF + replicar(LINEA_HO[0], 15) + CRUZ_INF + replicar(LINEA_HO[0], 15) + CRUZ_INF + replicar(LINEA_HO[0], 14) + ES_IN_DE, 14, 1);
        textoColor(startX + 15, startY + 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    
    int y = startY;
    
    // CABECERA DE LA TABLA
    textoColor(startX + 10, y++, ES_SU_IZ + replicar(LINEA_HO[0], 58) + ES_SU_DE, 14, 1);
    textoColor(startX + 10, y++, LINEA_VE + "              " + titulo + "               " + LINEA_VE, 14, 1);
    textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 12) + CRUZ_SUP + replicar(LINEA_HO[0], 15) + CRUZ_SUP + replicar(LINEA_HO[0], 15) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_DER, 14, 1);
    
    // ENCABEZADO DE COLUMNAS (CON DOCTOR)
    textoColor(startX + 12, y, "Fecha/Hora", 14, 1);
    textoColor(startX + 25, y, "Diagnostico", 14, 1);
    textoColor(startX + 42, y, "Tratamiento", 14, 1);
    textoColor(startX + 59, y, "Doctor", 14, 1);

    // Líneas verticales del encabezado
    textoColor(startX + 10, y, LINEA_VE, 14, 1);
    textoColor(startX + 23, y, LINEA_VE, 14, 1);
    textoColor(startX + 40, y, LINEA_VE, 14, 1);
    textoColor(startX + 57, y, LINEA_VE, 14, 1);
    textoColor(startX + 69, y, LINEA_VE, 14, 1);
    y++;
    
    textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 12) + CRUZ_CEN + replicar(LINEA_HO[0], 15) + CRUZ_CEN + replicar(LINEA_HO[0], 15) + CRUZ_CEN + replicar(LINEA_HO[0], 14) + CRUZ_DER, 14, 1);
    
    // LISTA DE CONSULTAS (del mas reciente al mas antiguo)
    for (int i = paciente->cantidadConsultas - 1; i >= 0; i--) {
        HistorialMedico* consulta = &paciente->historial[i];
        
        // Obtener doctor
        Doctor* doctor = buscarDoctorPorId(hospital, consulta->idDoctor);
        string apellidoDoctor = "N/E"; // No encontrado
        if (doctor != nullptr) {
            apellidoDoctor = doctor->apellido;
            // Acortar apellido si es muy largo
            if (apellidoDoctor.length() > 10) {
                apellidoDoctor = apellidoDoctor.substr(0, 8) + ".";
            }
        }
        
        // Fecha y hora juntas 
        string fechaHora = string(consulta->fecha).substr(5, 5) + " " + string(consulta->hora); 
        
        // Acortar diagnostico 
        string diagnosticoAbreviado = consulta->diagnostico;
        if (diagnosticoAbreviado.length() > 13) {
            diagnosticoAbreviado = diagnosticoAbreviado.substr(0, 11) + "..";
        }
        
        // Acortar tratamiento (incluye medicina si cabe)
        string tratamientoAbreviado = consulta->tratamiento;
        string medicamentos = consulta->medicamentos;
        
        // Si hay medicamentos, agregarlos al tratamiento
        if (!medicamentos.empty() && medicamentos != "-") {
            if (tratamientoAbreviado.length() + medicamentos.length() + 3 <= 13) {
                tratamientoAbreviado = tratamientoAbreviado + " (" + medicamentos + ")";
            } else {
                // Acortar tratamiento solo
                if (tratamientoAbreviado.length() > 13) {
                    tratamientoAbreviado = tratamientoAbreviado.substr(0, 11) + "..";
                }
            }
        } else {
            // Solo tratamiento
            if (tratamientoAbreviado.length() > 13) {
                tratamientoAbreviado = tratamientoAbreviado.substr(0, 11) + "..";
            }
        }
        
        // Alternar colores
        int colorFila = (i % 2 == 0) ? 15 : 7;
        
        // Línea lateral izquierda
        textoColor(startX + 10, y, LINEA_VE, 14, 1);
        
        // Fecha/Hora (juntas)
        textoColor(startX + 12, y, fechaHora, colorFila, 1);
        
        // diagnostico ( corto)
        textoColor(startX + 25, y, diagnosticoAbreviado, colorFila, 1);
        
        // Tratamiento (con medicina si cabe)
        textoColor(startX + 42, y, tratamientoAbreviado, colorFila, 1);
        
        // Doctor (apellido)
        textoColor(startX + 59, y, apellidoDoctor, colorFila, 1);
        
        // Líneas verticales
        textoColor(startX + 23, y, LINEA_VE, 14, 1);
        textoColor(startX + 40, y, LINEA_VE, 14, 1);
        textoColor(startX + 57, y, LINEA_VE, 14, 1);
        textoColor(startX + 69, y, LINEA_VE, 14, 1);
        
        y++;
        
        // Paginación (cada 20 consultas)
        if (y >= (startY + 20) && i > 0) {
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 12) + CRUZ_INF + replicar(LINEA_HO[0], 15) + CRUZ_INF + replicar(LINEA_HO[0], 15) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_DER, 14, 1);
            textoColor(startX + 15, y++, "Presione cualquier tecla para ver mas...", 14, 1);
            _getch();
            
            // Nueva pagina
            system("cls");
            system("color 1F");
            y = startY;
            textoColor(startX + 10, y++, ES_SU_IZ + replicar(LINEA_HO[0], 58) + ES_SU_DE, 14, 1);
            textoColor(startX + 10, y++, LINEA_VE + "              " + titulo + " (Cont.)          " + LINEA_VE, 14, 1);
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 12) + CRUZ_SUP + replicar(LINEA_HO[0], 15) + CRUZ_SUP + replicar(LINEA_HO[0], 15) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_DER, 14, 1);
            
            // Re-dibujar encabezado
            textoColor(startX + 12, y, "Fecha/Hora", 14, 1);
            textoColor(startX + 25, y, "Diagnostico", 14, 1);
            textoColor(startX + 42, y, "Tratamiento", 14, 1);
            textoColor(startX + 59, y, "Doctor", 14, 1);
            
            textoColor(startX + 10, y, LINEA_VE, 14, 1);
            textoColor(startX + 23, y, LINEA_VE, 14, 1);
            textoColor(startX + 40, y, LINEA_VE, 14, 1);
            textoColor(startX + 57, y, LINEA_VE, 14, 1);
            textoColor(startX + 69, y, LINEA_VE, 14, 1);
            y++;
            
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 12) + CRUZ_CEN + replicar(LINEA_HO[0], 15) + CRUZ_CEN + replicar(LINEA_HO[0], 15) + CRUZ_CEN + replicar(LINEA_HO[0], 14) + CRUZ_DER, 14, 1);
        }
    }
    
    // PIE DE TABLA
    textoColor(startX + 10, y++, ES_IN_IZ + replicar(LINEA_HO[0], 12) + CRUZ_INF + replicar(LINEA_HO[0], 15) + CRUZ_INF + replicar(LINEA_HO[0], 15) + CRUZ_INF + replicar(LINEA_HO[0], 14) + ES_IN_DE, 14, 1);
    textoColor(startX + 15, y++, "Total consultas: " + to_string(paciente->cantidadConsultas), 14, 1);
    textoColor(startX + 15, y++, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}


//---------------------------------------------
// MOSTRAR HISTORIAL MEDICO (INTERFAZ DE USUARIO)
//---------------------------------------------
void mostrarHistorialMedicoVisual(Hospital* hospital) {
    if (hospital == nullptr || hospital->cantidadPacientes == 0) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "HISTORIAL MEDICO", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    
    textoColor(25, 3, "HISTORIAL MEDICO", 14, 1);
    textoColor(10, 5, "Ingrese la cedula del paciente: ", 15, 1);
    textoColor(10, 7, "Presione ESC para cancelar", 14, 1);
    
    // Leer cedula del paciente
    string cedula = leerCampo(42, 5, 19);
    
    if (cedula == "ESC") {
        return; 
    }
    
    if (cedula.empty()) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "HISTORIAL MEDICO", 14, 1);
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
        textoColor(25, 3, "HISTORIAL MEDICO", 14, 1);
        textoColor(10, 5, "Error: No se encontro paciente con cedula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    if (!paciente->activo) {
        system("cls");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "HISTORIAL MEDICO", 14, 1);
        textoColor(10, 5, "Error: El paciente esta inactivo en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    string titulo = "HISTORIAL - " + string(paciente->nombre) + " " + string(paciente->apellido);
    mostrarHistorialMedicoEnTabla(hospital, paciente, titulo);
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
    hospital->capacidadPacientes = 10;  
    hospital->pacientes = new Paciente[hospital->capacidadPacientes];
    hospital->cantidadPacientes = 0;
    
    // PASO 4: Inicializar array de doctores
    hospital->capacidadDoctores = 10;   
    hospital->doctores = new Doctor[hospital->capacidadDoctores];
    hospital->cantidadDoctores = 0;
    
    // PASO 5: Inicializar array de citas
    hospital->capacidadCitas = 20;      
    hospital->citas = new Cita[hospital->capacidadCitas];
    hospital->cantidadCitas = 0;
    
    // PASO 6: Inicializar contadores de IDs 
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
    
    // PASO 1: Liberar memoria de todos los pacientes y sus arrays dinamicos
    for (int i = 0; i < hospital->cantidadPacientes; i++) {
        Paciente* paciente = &hospital->pacientes[i];
        
        // Liberar array dinamico de historial medico del paciente
        if (paciente->historial != nullptr) {
            delete[] paciente->historial;
            paciente->historial = nullptr;
        }
        
        // Liberar array dinamico de citas agendadas del paciente
        if (paciente->citasAgendadas != nullptr) {
            delete[] paciente->citasAgendadas;
            paciente->citasAgendadas = nullptr;
        }
    }
    
    // PASO 2: Liberar array dinamico de pacientes del hospital
    if (hospital->pacientes != nullptr) {
        delete[] hospital->pacientes;
        hospital->pacientes = nullptr;
    }
    
    // PASO 3: Liberar memoria de todos los doctores y sus arrays dinamicos
    for (int i = 0; i < hospital->cantidadDoctores; i++) {
        Doctor* doctor = &hospital->doctores[i];
        
        // Liberar array dinamico de pacientes asignados del doctor
        if (doctor->pacientesAsignados != nullptr) {
            delete[] doctor->pacientesAsignados;
            doctor->pacientesAsignados = nullptr;
        }
        
        // Liberar array dinamico de citas agendadas del doctor
        if (doctor->citasAgendadas != nullptr) {
            delete[] doctor->citasAgendadas;
            doctor->citasAgendadas = nullptr;
        }
    }
    
    // PASO 4: Liberar array dinamico de doctores del hospital
    if (hospital->doctores != nullptr) {
        delete[] hospital->doctores;
        hospital->doctores = nullptr;
    }
    
    // PASO 5: Liberar array dinamico de citas del hospital
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
    
    // Arrays para guardar IDs
    int idsDoctores[20];
    int idsPacientes[20];
    int idsCitas[20];
    
    // CREAR 20 DOCTORES
    idsDoctores[0] = crearDoctor(hospital, "Elena", "Gomez", "DR-01", "Cardiologia", 10, 80.0, "0412-1111111", "carlos@hospital.com")->id;
    idsDoctores[1] = crearDoctor(hospital, "Luisa", "Valbuena", "DR-02", "Pediatria", 8, 70.0, "0414-2222222", "maria@hospital.com")->id;
    idsDoctores[2] = crearDoctor(hospital, "German", "Martinez", "DR-03", "Traumatologia", 12, 90.0, "0416-3333333", "juan@hospital.com")->id;
    idsDoctores[3] = crearDoctor(hospital, "Albis", "Medina", "DR-04", "Dermatologia", 7, 75.0, "0424-4444444", "ana@hospital.com")->id;
    idsDoctores[4] = crearDoctor(hospital, "Ivanna", "Rodriguez", "DR-05", "Neurologia", 15, 100.0, "0412-5555555", "luis@hospital.com")->id;
    idsDoctores[5] = crearDoctor(hospital, "Angelica", "Marin", "DR-06", "Ginecologia", 9, 85.0, "0414-6666666", "elena@hospital.com")->id;
    idsDoctores[6] = crearDoctor(hospital, "Luis", "Govea", "DR-07", "Oftalmologia", 11, 80.0, "0416-7777777", "pedro@hospital.com")->id;
    idsDoctores[7] = crearDoctor(hospital, "David", "Sanchez", "DR-08", "Psiquiatria", 14, 95.0, "0424-8888888", "laura@hospital.com")->id;
    idsDoctores[8] = crearDoctor(hospital, "Jose", "Arrieta", "DR-09", "Cirugia", 18, 120.0, "0412-9999999", "miguel@hospital.com")->id;
    idsDoctores[9] = crearDoctor(hospital, "Yolbir", "Garcia", "DR-10", "Endocrinologia", 6, 70.0, "0414-1010101", "sofia@hospital.com")->id;
    idsDoctores[10] = crearDoctor(hospital, "Willie", "Linares", "DR-11", "Urologia", 13, 90.0, "0416-1111111", "jorge@hospital.com")->id;
    idsDoctores[11] = crearDoctor(hospital, "Juan", "Vargas", "DR-12", "Oncologia", 16, 110.0, "0424-1212121", "carmen@hospital.com")->id;
    idsDoctores[12] = crearDoctor(hospital, "Camila", "Castro", "DR-13", "Ortopedia", 10, 85.0, "0412-1313131", "ricardo@hospital.com")->id;
    idsDoctores[13] = crearDoctor(hospital, "Antonio", "Chavez", "DR-14", "Pediatria", 5, 65.0, "0414-1414141", "patricia@hospital.com")->id;
    idsDoctores[14] = crearDoctor(hospital, "Marie", "Mendoza", "DR-15", "Cardiologia", 11, 85.0, "0416-1515151", "fernando@hospital.com")->id;
    idsDoctores[15] = crearDoctor(hospital, "Ramon", "Colina", "DR-16", "Dermatologia", 8, 75.0, "0424-1616161", "diana@hospital.com")->id;
    idsDoctores[16] = crearDoctor(hospital, "Carlos", "Machado", "DR-17", "Neurologia", 12, 95.0, "0412-1717171", "roberto@hospital.com")->id;
    idsDoctores[17] = crearDoctor(hospital, "Lupe", "Barreto", "DR-18", "Ginecologia", 7, 80.0, "0414-1818181", "gabriela@hospital.com")->id;
    idsDoctores[18] = crearDoctor(hospital, "Sara", "Ardila", "DR-19", "Traumatologia", 9, 85.0, "0416-1919191", "alejandro@hospital.com")->id;
    idsDoctores[19] = crearDoctor(hospital, "Carmen", "Quintero", "DR-20", "Psiquiatria", 10, 85.0, "0424-2020202", "teresa@hospital.com")->id;
    
    // CREAR 20 PACIENTES
    idsPacientes[0] = crearPaciente(hospital, "Ghiber", "Linares", "PA-01", 18, 'M', "O+", "Penicilina", "0414-0705771", "Av Principal", "ghiber@gmail.com")->id;
    idsPacientes[1] = crearPaciente(hospital, "Ana", "Gonzalez", "PA-02", 25, 'F', "A+", "Ninguna", "0412-1111111", "Calle 1", "ana@email.com")->id;
    idsPacientes[2] = crearPaciente(hospital, "Carlos", "Lopez", "PA-03", 30, 'M', "B+", "Mariscos", "0414-2222222", "Calle 2", "carlos@email.com")->id;
    idsPacientes[3] = crearPaciente(hospital, "Maria", "Martinez", "PA-04", 22, 'F', "AB+", "Polen", "0416-3333333", "Calle 3", "maria@email.com")->id;
    idsPacientes[4] = crearPaciente(hospital, "Jose", "Rodriguez", "PA-05", 35, 'M', "O-", "Aspirina", "0424-4444444", "Calle 4", "jose@email.com")->id;
    idsPacientes[5] = crearPaciente(hospital, "Laura", "Sanchez", "PA-06", 28, 'F', "A-", "Ninguna", "0412-5555555", "Calle 5", "laura@email.com")->id;
    idsPacientes[6] = crearPaciente(hospital, "Miguel", "Hernandez", "PA-07", 40, 'M', "B-", "Polvo", "0414-6666666", "Calle 6", "miguel@email.com")->id;
    idsPacientes[7] = crearPaciente(hospital, "Elena", "Diaz", "PA-08", 32, 'F', "AB-", "Acaros", "0416-7777777", "Calle 7", "elena@email.com")->id;
    idsPacientes[8] = crearPaciente(hospital, "Pedro", "Torres", "PA-09", 45, 'M', "O+", "Latex", "0424-8888888", "Calle 8", "pedro@email.com")->id;
    idsPacientes[9] = crearPaciente(hospital, "Sofia", "Ramirez", "PA-10", 26, 'F', "A+", "Yodo", "0412-9999999", "Calle 9", "sofia@email.com")->id;
    idsPacientes[10] = crearPaciente(hospital, "Andres", "Flores", "PA-11", 33, 'M', "B+", "Ninguna", "0414-1010101", "Calle 10", "andres@email.com")->id;
    idsPacientes[11] = crearPaciente(hospital, "Carmen", "Vargas", "PA-12", 29, 'F', "AB+", "Penicilina", "0416-1111111", "Calle 11", "carmen@email.com")->id;
    idsPacientes[12] = crearPaciente(hospital, "Ricardo", "Castro", "PA-13", 38, 'M', "O-", "Mariscos", "0424-1212121", "Calle 12", "ricardo@email.com")->id;
    idsPacientes[13] = crearPaciente(hospital, "Patricia", "Rojas", "PA-14", 24, 'F', "A-", "Polen", "0412-1313131", "Calle 13", "patricia@email.com")->id;
    idsPacientes[14] = crearPaciente(hospital, "Fernando", "Mendoza", "PA-15", 42, 'M', "B-", "Aspirina", "0414-1414141", "Calle 14", "fernando@email.com")->id;
    idsPacientes[15] = crearPaciente(hospital, "Diana", "Ortega", "PA-16", 27, 'F', "AB-", "Ninguna", "0416-1515151", "Calle 15", "diana@email.com")->id;
    idsPacientes[16] = crearPaciente(hospital, "Roberto", "Silva", "PA-17", 36, 'M', "O+", "Polvo", "0424-1616161", "Calle 16", "roberto@email.com")->id;
    idsPacientes[17] = crearPaciente(hospital, "Gabriela", "Morales", "PA-18", 31, 'F', "A+", "Acaros", "0412-1717171", "Calle 17", "gabriela@email.com")->id;
    idsPacientes[18] = crearPaciente(hospital, "Alejandro", "Reyes", "PA-19", 39, 'M', "B+", "Latex", "0414-1818181", "Calle 18", "alejandro@email.com")->id;
    idsPacientes[19] = crearPaciente(hospital, "Teresa", "Perez", "PA-20", 23, 'F', "AB+", "Yodo", "0416-1919191", "Calle 19", "teresa@email.com")->id;
    
    // CREAR 20 CITAS
    idsCitas[0] = agendarCita(hospital, idsPacientes[0], idsDoctores[0], "2025-01-15", "09:00", "Consulta cardiologica")->id;
    idsCitas[1] = agendarCita(hospital, idsPacientes[1], idsDoctores[1], "2025-01-16", "10:00", "Control pediatrico")->id;
    idsCitas[2] = agendarCita(hospital, idsPacientes[2], idsDoctores[2], "2025-01-17", "11:00", "Dolor rodilla")->id;
    idsCitas[3] = agendarCita(hospital, idsPacientes[3], idsDoctores[3], "2025-01-18", "12:00", "Erupcion cutanea")->id;
    idsCitas[4] = agendarCita(hospital, idsPacientes[4], idsDoctores[4], "2025-01-19", "13:00", "Dolor cabeza")->id;
    idsCitas[5] = agendarCita(hospital, idsPacientes[5], idsDoctores[5], "2025-01-20", "14:00", "Consulta ginecologica")->id;
    idsCitas[6] = agendarCita(hospital, idsPacientes[6], idsDoctores[6], "2025-01-21", "15:00", "Examen vista")->id;
    idsCitas[7] = agendarCita(hospital, idsPacientes[7], idsDoctores[7], "2025-01-22", "16:00", "Consulta psicologica")->id;
    idsCitas[8] = agendarCita(hospital, idsPacientes[8], idsDoctores[8], "2025-01-23", "09:30", "Evaluacion cirugia")->id;
    idsCitas[9] = agendarCita(hospital, idsPacientes[9], idsDoctores[9], "2025-01-24", "10:30", "Control hormonal")->id;
    idsCitas[10] = agendarCita(hospital, idsPacientes[10], idsDoctores[10], "2025-01-25", "11:30", "Consulta urologica")->id;
    idsCitas[11] = agendarCita(hospital, idsPacientes[11], idsDoctores[11], "2025-01-26", "12:30", "Seguimiento oncologico")->id;
    idsCitas[12] = agendarCita(hospital, idsPacientes[12], idsDoctores[12], "2025-01-27", "13:30", "Dolor espalda")->id;
    idsCitas[13] = agendarCita(hospital, idsPacientes[13], idsDoctores[13], "2025-01-28", "14:30", "Control nino")->id;
    idsCitas[14] = agendarCita(hospital, idsPacientes[14], idsDoctores[14], "2025-01-29", "15:30", "Presion alta")->id;
    idsCitas[15] = agendarCita(hospital, idsPacientes[15], idsDoctores[15], "2025-01-30", "16:30", "Acne")->id;
    idsCitas[16] = agendarCita(hospital, idsPacientes[16], idsDoctores[16], "2025-02-01", "09:00", "Mareos")->id;
    idsCitas[17] = agendarCita(hospital, idsPacientes[17], idsDoctores[17], "2025-02-02", "10:00", "Consulta ginecologica")->id;
    idsCitas[18] = agendarCita(hospital, idsPacientes[18], idsDoctores[18], "2025-02-03", "11:00", "Lesion deportiva")->id;
    idsCitas[19] = agendarCita(hospital, idsPacientes[19], idsDoctores[19], "2025-02-04", "12:00", "Ansiedad")->id;
    
    // ATENDER 5 CITAS
    atenderCita(hospital, idsCitas[0], "Hipertension grado 1", "Dieta baja en sal, ejercicio", "Losartan 50mg");
    atenderCita(hospital, idsCitas[1], "Control crecimiento normal", "Seguimiento rutinario", "Multivitaminico");
    atenderCita(hospital, idsCitas[2], "Esguince tobillo", "Reposo, hielo, elevacion", "Ibuprofeno 400mg");
    atenderCita(hospital, idsCitas[3], "Dermatitis contacto", "Crema hidratante", "Hidrocortisona 1%");
    atenderCita(hospital, idsCitas[4], "Migrana tensional", "Manejo estres", "Paracetamol 500mg");
    
    // MENSAJE DE CONFIRMACION
    system("cls");
    dibujarCuadro(2, 3, 75, 10);
    textoColor(25, 3, "DATOS DE PRUEBA GENERADOS", 14, 1);
    textoColor(10, 5, "20 Doctores creados", 10, 1);
    textoColor(10, 6, "20 Pacientes creados", 10, 1);
    textoColor(10, 7, "20 Citas agendadas", 10, 1);
    textoColor(10, 8, "5 Citas atendidas", 10, 1);
    textoColor(10, 9, "Presione cualquier tecla...", 14, 1);
    _getch();
}

// MENU PRINCIPAL
//---------------------------------------------
int mostrarMenuPrincipal(Hospital* hospital) {
    system("cls");
    system("color 1F"); // Fondo azul, texto blanco
    dibujarCuadro(5, 2, 70, 17);
    
    // Titulo principal en amarillo
    textoColor(23, 1, "SISTEMA DE GESTION HOSPITALARIA", 14, 1);
    
    // Nombre del hospital en verde claro
    textoColor(28, 3, hospital->nombre, 10, 1);
    
    // Opciones del menú en blanco
    textoColor(20, 5, "1. Gestion de Pacientes", 15, 1);
    textoColor(20, 6, "2. Gestion de Doctores", 15, 1);
    textoColor(20, 7, "3. Gestion de Citas", 15, 1);
    textoColor(20, 8, "ESC. Salir", 12, 1); // Salir en rojo

    // Linea separadora en gris
    textoColor(10, 11, "================================================", 7, 1);
    
    // ESTADISTICAS con colores
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
                mostrarHistorialMedicoVisual(hospital);
                break;
            case 5:
                actualizarPaciente(hospital);
                break;
            case 6:
                listarPacientes(hospital);
                break;
            case 7:
                eliminarPacienteVisual(hospital);
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
        
        // Titulo en amarillo
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
                buscarDoctorPorIdVisual(hospital);
                break;
            case 3:
                buscarDoctoresPorEspecialidadVisual(hospital); 
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
                eliminarDoctorVisual(hospital);
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
                agendarCitaVisual(hospital);
                break;
            case 2:
                cancelarCita(hospital, 1);
                break;
            case 3:
                atenderCitaVisual(hospital);
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
