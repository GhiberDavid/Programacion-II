#include <iostream>
#include <fstream>
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib> 
#include <ctime> 


using namespace std;


// CARACTERES QUE FUNCIONAN EN WINDOWS
const string ES_SU_IZ = "\xDA";      // +
const string ES_SU_DE = "\xBF";      // +  
const string ES_IN_IZ = "\xC0";      // +
const string ES_IN_DE = "\xD9";      // +
const string LINEA_HO = "\xC4";      // -
const string LINEA_VE = "\xB3";       // ¦
const string CRUZ_SUP = "\xC2";         // -
const string CRUZ_INF = "\xC1";         // -
const string CRUZ_IZQ = "\xC3";         // ¦
const string CRUZ_DER = "\xB4";         // ¦
const string CRUZ_CEN = "\xC5";         // +

//---------------------------------------------
// DECLARACIONES DE ESTRUCTURAS 
//---------------------------------------------


// Header para archivos binarios
struct ArchivoHeader {
    int cantidadRegistros;
    int proximoID;
    int registrosActivos;
    int version;
};



struct Cita {
    int id;
    int idPaciente;
    int idDoctor;
    char fecha[11];        // DD-MM-YYYY
    char hora[6];          // HH:MM
    char motivo[150];
    char estado[20];       // "Agendada", "Atendida", "Cancelada"
    char observaciones[200];
    bool atendida;
    
    // NUEVO: Metadata para archivos binarios
    bool eliminada;                 // Borrado lógico
    time_t fechaCreacion;           // Fecha de creación
    time_t fechaModificacion;       // Última modificación
    int idHistorialAsociado;        // ID del historial médico si existe
};

struct HistorialMedico {
    int id;
    char fecha[11];        // DD-MM-YYYY
    char hora[6];          // HH:MM
    char diagnostico[200];
    char tratamiento[200];
    char medicamentos[150];
    int idDoctor;
    float costo;
    
    // NUEVO: Metadata para archivos binarios
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaModificacion;
    int idConsulta;    // ID de la cita que generó este historial
    int idPaciente;        // ID del paciente
};



struct Paciente {
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[20];       // DEBE SER UNICA
    int edad;              // Validar: 0-120 aÃ±os
    char sexo;             // 'M' o 'F'
    char tipoSangre[5];    // "O+", "A-", "AB+", etc.
    char telefono[15];
    char direccion[100];
    char email[50];
    
    // Array dinamico de historial medico
    HistorialMedico* historial;
    int capacidadHistorial;  // Capacidad inicial: 5
    
    // Array dinamico de IDs de citas
    int* citasAgendadas;
    
    char alergias[500];      // "Alergia1, Alergia2, ..."
    char observaciones[500]; // Notas generales
    bool activo;
    
    
    // NUEVO: Índices para relaciones
    int cantidadConsultas;
    int primerConsultaID;
    int cantidadCitas;
    int citasIDs[20];
    
    // Metadata de registro
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaModificacion;    
    
    
};

struct Doctor {
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[20];  // DEBE SER ÚNICA
    char especialidad[50];        // "Cardiología", "Pediatría", etc.
    int aniosExperiencia;        // Validar: mínimo 0
    float costoConsulta;         // Validar: mayor a 0
    char horarioAtencion[50];    // "Lun-Vie 8:00-16:00"
    char telefono[15];
    char email[50];
    
    // NUEVO: Relaciones con arrays fijos (reemplazo de arrays dinámicos)
    int cantidadPacientes;
    int pacientesIDs[50];           // Máximo 50 pacientes asignados
    int cantidadCitas;
    int citasIDs[30];               // Máximo 30 citas agendadas
    
    bool disponible;               // Disponible para nuevos pacientes
    bool activo;    
    
    // NUEVO: Metadata para persistencia
    bool eliminado;                // Borrado lógico
    time_t fechaCreacion;
    time_t fechaModificacion;
};

struct Hospital {
    char nombre[100];
    char direccion[150];
    char telefono[15];
    
    // Contadores de IDs (auto-increment)
    int siguienteIDPaciente;
    int siguienteIDDoctor;
    int siguienteIDCita;
    int siguienteIDConsulta;
    
    // Estadísticas generales
    int totalPacientesRegistrados;
    int totalDoctoresRegistrados;
    int totalCitasAgendadas;
    int totalConsultasRealizadas;
    
    // METADATOS PARA ARCHIVOS (opcionales, pero recomendados)
    bool archivosInicializados;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
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
            // Borrar carÃ¡cter con fondo azul
            textoColor(x + texto.size(), y, " ", 15, 1);
        }
        else if (isprint((unsigned char)c) && texto.size() < maxlen) {
            texto.push_back(c);
            // Escribir carÃ¡cter con color correcto
            textoColor(x + texto.size() - 1, y, string(1, c), 15, 1);
        }
    }
    return texto;
}

string leerCampoNumerico(int x, int y, int maxlen, const string& valorInicial = "", bool soloLectura = false) {
    string texto = valorInicial;
    char c;
    
    // Limpiar Ã¡rea antes de leer (fondo azul, texto blanco)
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
        
        if (c == 13) { // Enter
            // No permitir Enter si el campo estÃ¡ vacÃ­o
            if (!texto.empty()) {
                break;
            }
            // Si estÃ¡ vacÃ­o, ignorar Enter y continuar
            continue;
        }
        if (c == 27) { // ESC - siempre permitido
            texto = "ESC";
            break;
        }
        if (c == 8 && !texto.empty()) { // Backspace
            texto.pop_back();
            // Borrar carÃ¡cter con fondo azul
            textoColor(x + texto.size(), y, " ", 15, 1);
            gotoxy(x + texto.size(), y);
        }
        else if (isdigit((unsigned char)c) && texto.size() < maxlen) {
            texto.push_back(c);
            // Escribir dÃ­gito con color correcto
            textoColor(x + texto.size() - 1, y, string(1, c), 15, 1);
            
            // Si maxlen es 1 y ya tenemos un dÃ­gito, salir automÃ¡ticamente
            if (maxlen == 1 && texto.size() == 1) {
                break;
            }
        }
    }
    return texto;
}

//---------------------------------------------
// FUNCIÃ“N AUXILIAR: COMPARAR STRINGS (Case-Insensitive)
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

// FUNCIÓN AUXILIAR: CONVERTIR STRING A MINÚSCULAS (C)
//---------------------------------------------
void stringToLower(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0' && i < 499) { // límite por seguridad
        dest[i] = tolower(src[i]);
        i++;
    }
    dest[i] = '\0';
}

//---------------------------------------------
// FUNCIÓN AUXILIAR: BUSCAR SUBSTRING (CASE-INSENSITIVE)
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
// VALIDAR FORMATO DE FECHA (DD-MM-YYYY)
//---------------------------------------------
bool validarFecha(const char* fecha) {
    if (fecha == NULL || strlen(fecha) != 10)
        return false;

    // Verificar formato DD-MM-YYYY
    if (fecha[2] != '-' || fecha[5] != '-')
        return false;

    // Verificar que los demÃ¡s caracteres sean dÃ­gitos
    for (int i = 0; i < 10; i++) {
        if (i != 2 && i != 5 && !isdigit((unsigned char)fecha[i]))
            return false;
    }

    // Extraer dia, mes y aÑO
    char diaStr[3], mesStr[3], anioStr[5];
    strncpy(diaStr, fecha, 2);    diaStr[2] = '\0';
    strncpy(mesStr, fecha + 3, 2); mesStr[2] = '\0';
    strncpy(anioStr, fecha + 6, 4); anioStr[4] = '\0';

    int dia = atoi(diaStr);
    int mes = atoi(mesStr);
    int anio = atoi(anioStr);

    // Validar aÃ±o (2020-2030)
    if (anio < 2020 || anio > 2030) return false;
    
    // Validar mes (1-12)
    if (mes < 1 || mes > 12) return false;
    
    // Validar dÃ­a (mÃ­nimo 1)
    if (dia < 1) return false;

    // DÃ­as por mes
    int diasMes[] = {31,28,31,30,31,30,31,31,30,31,30,31};

    // AÃ±o bisiesto
    bool bisiesto = (anio % 4 == 0 && (anio % 100 != 0 || anio % 400 == 0));
    if (bisiesto && mes == 2) diasMes[1] = 29;

    // Validar dÃ­a segÃºn el mes
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
    
    // Verificar que sean dígitos
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
    
    // Mostrar instrucción para cancelar
    textoColor(x, y, "Presione ESC en cualquier campo para cancelar", 14, 1);
}

void mostrarMensajeExito(int x, int y, const char* mensaje) {
    // Limpiar Texto Anterior
    textoColor(x, y, replicar(' ', 60), 10, 1);

    // Mostrar mensaje de éxito (color verde)
    textoColor(x, y, mensaje, 10, 1);
    _getch();
    
    // Limpiar el mensaje después de presionar una tecla
    textoColor(x, y, replicar(' ', 60), 10, 1);
}

// Leer header de pacientes
ArchivoHeader leerHeaderPacientes() {
    ArchivoHeader header;
    ifstream archivo("pacientes.bin", ios::binary);
    
    if (archivo.is_open() && archivo.read((char*)&header, sizeof(ArchivoHeader))) {
        archivo.close();
    } else {
        // Si no existe o no se puede leer, inicializar valores por defecto
        header.cantidadRegistros = 0;
        header.proximoID = 1;
        header.registrosActivos = 0;
        header.version = 1;
    }
    
    return header;
}

//---------------------------------------------
// LEER HEADER DE DOCTORES
//---------------------------------------------
ArchivoHeader leerHeaderDoctores() {
    ArchivoHeader header;
    ifstream archivo("doctores.bin", ios::binary);
    
    if (archivo.is_open() && archivo.read((char*)&header, sizeof(ArchivoHeader))) {
        archivo.close();
    } else {
        // Si no existe o no se puede leer, inicializar valores por defecto
        header.cantidadRegistros = 0;
        header.proximoID = 1;
        header.registrosActivos = 0;
        header.version = 1;
    }
    
    return header;
}

// Leer header de citas
ArchivoHeader leerHeaderCitas() {
    ArchivoHeader header;
    ifstream archivo("citas.bin", ios::binary);
    
    if (archivo.is_open() && archivo.read((char*)&header, sizeof(ArchivoHeader))) {
        archivo.close();
    } else {
        // Si no existe o no se puede leer, inicializar valores por defecto
        header.cantidadRegistros = 0;
        header.proximoID = 1;
        header.registrosActivos = 0;
        header.version = 1;
    }
    
    return header;
}

// Leer header de historial médico
ArchivoHeader leerHeaderConsultas() {
    ArchivoHeader header;
    ifstream archivo("historial.bin", ios::binary);
    
    if (archivo.is_open() && archivo.read((char*)&header, sizeof(ArchivoHeader))) {
        archivo.close();
    } else {
        // Si no existe o no se puede leer, inicializar valores por defecto
        header.cantidadRegistros = 0;
        header.proximoID = 1;
        header.registrosActivos = 0;
        header.version = 1;
    }
    
    return header;
}

// Inicializar archivo de pacientes
bool inicializarArchivoPacientes() {
    fstream archivo("pacientes.bin", ios::binary | ios::out);
    if (!archivo.is_open()) return false;
    
    ArchivoHeader header;
    header.cantidadRegistros = 0;
    header.proximoID = 1;
    header.registrosActivos = 0;
    header.version = 1;
    
    archivo.write((char*)&header, sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool inicializarArchivoDoctores() {
    fstream archivo("doctores.bin", ios::binary | ios::out);
    if (!archivo.is_open()) return false;
    
    ArchivoHeader header;
    header.cantidadRegistros = 0;
    header.proximoID = 1;
    header.registrosActivos = 0;
    header.version = 1;
    
    archivo.write((char*)&header, sizeof(ArchivoHeader));
    archivo.close();
    return true;
}


// Inicializar archivo de citas
bool inicializarArchivoCitas() {
    fstream archivo("citas.bin", ios::binary | ios::out);
    if (!archivo.is_open()) return false;
    
    ArchivoHeader header;
    header.cantidadRegistros = 0;
    header.proximoID = 1;
    header.registrosActivos = 0;
    header.version = 1;
    
    archivo.write((char*)&header, sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

// Inicializar archivo de historial médico
bool inicializarArchivoHistorial() {
    fstream archivo("historial.bin", ios::binary | ios::out);
    if (!archivo.is_open()) return false;
    
    ArchivoHeader header;
    header.cantidadRegistros = 0;
    header.proximoID = 1;
    header.registrosActivos = 0;
    header.version = 1;
    
    archivo.write((char*)&header, sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

//---------------------------------------------
// FUNCIONES DE HOSPITAL 
//---------------------------------------------



// 1. CREAR/INICIALIZAR HOSPITAL
bool crearHospital(const char* nombre, const char* direccion, const char* telefono) {
    Hospital hosp;
    
    // Configurar información básica
    strncpy(hosp.nombre, nombre, 99);
    strncpy(hosp.direccion, direccion, 149);
    strncpy(hosp.telefono, telefono, 14);
    
    // Inicializar contadores (comienzan en 1)
    hosp.siguienteIDPaciente = 1;
    hosp.siguienteIDDoctor = 1;
    hosp.siguienteIDCita = 1;
    hosp.siguienteIDConsulta = 1;
    
    // Inicializar estadísticas
    hosp.totalPacientesRegistrados = 0;
    hosp.totalDoctoresRegistrados = 0;
    hosp.totalCitasAgendadas = 0;
    hosp.totalConsultasRealizadas = 0;
    
    // Inicializar metadata
    hosp.archivosInicializados = false;
    time_t ahora = time(0);
    hosp.fechaCreacion = ahora;
    hosp.fechaUltimaModificacion = ahora;
    
    // Guardar en archivo
    ofstream archivo("hospital.bin", ios::binary);
    if (!archivo.is_open()) return false;
    
    archivo.write((char*)&hosp, sizeof(Hospital));
    archivo.close();
    
    // Inicializar los demás archivos
    inicializarArchivoPacientes();
    inicializarArchivoDoctores();
    inicializarArchivoCitas();
    inicializarArchivoHistorial();
    
    return true;
}

// 2. CARGAR HOSPITAL DESDE ARCHIVO
Hospital cargarHospital() {
    Hospital hosp;
    ifstream archivo("hospital.bin", ios::binary);
    
    if (archivo.is_open()) {
        archivo.read((char*)&hosp, sizeof(Hospital));
        archivo.close();
    } else {
        // Si no existe, crear uno por defecto
        crearHospital("Hospital Central", "Av. Principal 123", "0261-1234567");
        hosp = cargarHospital(); // Recursivo para cargar el recién creado
    }
    
    return hosp;
}

// 3. GUARDAR HOSPITAL EN ARCHIVO
bool guardarHospital(Hospital& hosp) {
    // Actualizar timestamp
    hosp.fechaUltimaModificacion = time(0);
    
    // Guardar
    ofstream archivo("hospital.bin", ios::binary);
    if (!archivo.is_open()) return false;
    
    archivo.write((char*)&hosp, sizeof(Hospital));
    archivo.close();
    
    return true;
}

// 4. SINCRONIZAR ESTADÍSTICAS CON ARCHIVOS
void sincronizarEstadisticas(Hospital& hosp) {
    // Leer estadísticas reales de los archivos
    ArchivoHeader headerPac = leerHeaderPacientes();
    ArchivoHeader headerDoc = leerHeaderDoctores();
    ArchivoHeader headerCit = leerHeaderCitas();
    ArchivoHeader headerHist = leerHeaderConsultas();
    
    // Actualizar contadores del hospital
    hosp.siguienteIDPaciente = headerPac.proximoID;
    hosp.siguienteIDDoctor = headerDoc.proximoID;
    hosp.siguienteIDCita = headerCit.proximoID;
    hosp.siguienteIDConsulta = headerHist.proximoID;
    
    // Actualizar estadísticas
    hosp.totalPacientesRegistrados = headerPac.registrosActivos;
    hosp.totalDoctoresRegistrados = headerDoc.registrosActivos;
    hosp.totalCitasAgendadas = headerCit.registrosActivos;
    hosp.totalConsultasRealizadas = headerHist.registrosActivos;
    
    // Guardar cambios
    guardarHospital(hosp);
}

// 6. FUNCIÓN PARA ACTUALIZAR CONTADORES AL AGREGAR REGISTROS
void actualizarContadorHospital(const string& tipo) {
    // Cargar el hospital actual desde archivo
    Hospital hosp = cargarHospital();
    
    if (tipo == "paciente") {
        hosp.siguienteIDPaciente++;
        hosp.totalPacientesRegistrados++;
    } 
    else if (tipo == "doctor") {
        hosp.siguienteIDDoctor++;
        hosp.totalDoctoresRegistrados++;
    }
    else if (tipo == "cita") {
        hosp.siguienteIDCita++;
        hosp.totalCitasAgendadas++;
    }
    else if (tipo == "consulta") {
        hosp.siguienteIDConsulta++;
        hosp.totalConsultasRealizadas++;
    }
    
    guardarHospital(hosp);
}



//---------------------------------------------
// FUNCIONES DE PACIENTES 
//---------------------------------------------



Paciente buscarPacientePorCedula(const char* cedula) {
    if (cedula == nullptr || strlen(cedula) == 0) {
        Paciente vacio;
        vacio.id = -1;
        return vacio;
    }
    
    // Abriendo archivo de pacientes
    ifstream archivo("pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        Paciente vacio;
        vacio.id = -1;
        return vacio;
    }
    
    // Leyendo header para saber cantidad de registros
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    // Saltando header y buscar en todos los pacientes
    archivo.seekg(sizeof(ArchivoHeader));
    
    Paciente temp;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Paciente));
        
        // Verificar que el paciente esté activo y comparar cédulas
        if (!temp.eliminado && temp.activo && compararStrings(temp.cedula, cedula)) {
            archivo.close();
            return temp; // Encontrado
        }
    }
    
    archivo.close();
    
    // No encontrado
    Paciente vacio;
    vacio.id = -1;
    return vacio;
}

//---------------------------------------------
// BUSCAR PACIENTES POR NOMBRE (PARCIAL, CASE-INSENSITIVE) 
//---------------------------------------------

Paciente* buscarPacientesPorNombre(const char* nombreBuscado, int* cantidadResultados) {
    *cantidadResultados = 0;
    
    if (nombreBuscado == nullptr || strlen(nombreBuscado) == 0) {
        return nullptr;
    }
    
    // Abriendo archivo de pacientes
    ifstream archivo("pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        return nullptr;
    }
    
    // Leyendo header para saber cuántos registros hay
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    if (header.cantidadRegistros == 0) {
        archivo.close();
        return nullptr;
    }
    
    //Contando cuántos pacientes coinciden
    Paciente temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    int contador = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Paciente));
        
        if (!temp.eliminado && temp.activo) {
            // Creando nombre completo temporal
            char nombreCompleto[101]; // nombre(50) + espacio + apellido(50) = 101
            strcpy(nombreCompleto, temp.nombre);
            strcat(nombreCompleto, " ");
            strcat(nombreCompleto, temp.apellido);
            
            if (contieneSubstringCaseInsensitive(nombreCompleto, nombreBuscado)) {
                contador++;
            }
        }
    }
    
    if (contador == 0) {
        archivo.close();
        *cantidadResultados = 0;
        return nullptr;
    }
    
    
    Paciente* resultados = new Paciente[contador];
    
    // Volviendo al inicio del archivo
    archivo.seekg(sizeof(ArchivoHeader));
    int index = 0;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Paciente));
        
        if (!temp.eliminado && temp.activo) {
            // Crear nombre completo temporal
            char nombreCompleto[101];
            strcpy(nombreCompleto, temp.nombre);
            strcat(nombreCompleto, " ");
            strcat(nombreCompleto, temp.apellido);
            
            if (contieneSubstringCaseInsensitive(nombreCompleto, nombreBuscado)) {
                resultados[index++] = temp;
            }
        }
    }
    
    archivo.close();
    *cantidadResultados = contador;
    return resultados;
}


//---------------------------------------------
// BUSCAR PACIENTE POR ID
//---------------------------------------------
Paciente buscarPacientePorID(int id) {
	
    ifstream archivo("pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        Paciente vacio;
        vacio.id = -1;
        return vacio;
    }
    
    // Leyendo header para saber cantidad de registros
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    // Saltando header: seekg(sizeof(ArchivoHeader))
    archivo.seekg(sizeof(ArchivoHeader));
    
    // Leyendo cada paciente hasta encontrar ID o EOF
    Paciente temp;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Paciente));
        if (temp.id == id && !temp.eliminado) {
            archivo.close();
            return temp; // Encontrado
        }
    }
    
    // 5. Retornar paciente vacío si no existe
    archivo.close();
    Paciente vacio;
    vacio.id = -1;
    return vacio;
}

//---------------------------------------------
// BUSCAR ÍNDICE DE PACIENTE POR ID
//---------------------------------------------
int buscarIndicePacientePorID(int id) {
	
    ifstream archivo("pacientes.bin", ios::binary);
    if (!archivo.is_open()) return -1;
    
    // Leer header
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    // Buscar en todos los registros
    Paciente temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Paciente));
        if (temp.id == id && !temp.eliminado) {
            archivo.close();
            return i;  // Retorna el índice (posición en el archivo)
        }
    }
    
    archivo.close();
    return -1;  // No encontrado
}


//---------------------------------------------
// CALCULAR POSICIÓN EN ARCHIVO
//---------------------------------------------
long calcularPosicionPaciente(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(Paciente));
}

bool actualizarPaciente(Paciente pacienteModificado) {
    // Buscar la posición del paciente en el archivo
    int indice = buscarIndicePacientePorID(pacienteModificado.id);
    if (indice == -1) return false;
    
    // Actualizar timestamp
    pacienteModificado.fechaModificacion = time(0);
    
    // Abrir archivo y guardar cambios
    fstream archivo("pacientes.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    
    long posicion = calcularPosicionPaciente(indice);
    archivo.seekp(posicion);
    archivo.write((char*)&pacienteModificado, sizeof(Paciente));
    archivo.close();
    
    return true;
}

//---------------------------------------------
// FUNCICION AUXILIAR: MOSTRAR Y LIMPIAR ERROR
//---------------------------------------------
void mostrarError(int x, int y, const string& mensaje) {
    textoColor(x, y, mensaje, 12, 1);
    _getch();
    textoColor(x, y, replicar(' ', mensaje.length()), 1, 1); // Limpiar mensaje
}



//---------------------------------------------
// CREAR PACIENTE 
//---------------------------------------------
Paciente crearPaciente(const char* nombre, const char* apellido, 
                      const char* cedula, int edad, char sexo, const char* tipoSangre, 
                      const char* alergias, const char* telefono, const char* direccion, 
                      const char* email) {
    
    Paciente nuevoPaciente;
    
    // INICIALIZAR EL PACIENTE (sin ID todavía - se asignará al guardar)
    nuevoPaciente.activo = true;
    
    // Copiar datos básicos
    strncpy(nuevoPaciente.nombre, nombre, 49);
    strncpy(nuevoPaciente.apellido, apellido, 49);
    strncpy(nuevoPaciente.cedula, cedula, 19);
    nuevoPaciente.edad = edad;
    nuevoPaciente.sexo = sexo;
    strncpy(nuevoPaciente.tipoSangre, tipoSangre, 4);
    strncpy(nuevoPaciente.alergias, alergias, 499);
    strncpy(nuevoPaciente.telefono, telefono, 14);
    strncpy(nuevoPaciente.direccion, direccion, 99);
    strncpy(nuevoPaciente.email, email, 49);
    
    // Inicializar campos vacíos
    strcpy(nuevoPaciente.observaciones, "");
    
    // Inicializar arrays fijos y contadores
    nuevoPaciente.cantidadConsultas = 0;
    nuevoPaciente.primerConsultaID = -1;  // Sin historial aún
    nuevoPaciente.cantidadCitas = 0;
    // El array citasIDs[20] ya está inicializado automáticamente
    
    // Metadata
    nuevoPaciente.eliminado = false;
    time_t now = time(0);
    nuevoPaciente.fechaCreacion = now;
    nuevoPaciente.fechaModificacion = now;
    
    return nuevoPaciente;
}


// Actualizar header de pacientes
bool actualizarHeaderPacientes(ArchivoHeader nuevoHeader) {
    fstream archivo("pacientes.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    
    archivo.seekp(0);
    archivo.write((char*)&nuevoHeader, sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool agregarPaciente(Paciente& nuevoPaciente) {

    ArchivoHeader header = leerHeaderPacientes();
    
    // Configurar datos del paciente
    nuevoPaciente.id = header.proximoID;
    nuevoPaciente.activo = true;
    nuevoPaciente.eliminado = false;
    nuevoPaciente.cantidadConsultas = 0;
    nuevoPaciente.primerConsultaID = -1;
    nuevoPaciente.cantidadCitas = 0;
    
    // Inicializar array de citas
    for (int i = 0; i < 20; i++) {
        nuevoPaciente.citasIDs[i] = -1;
    }
    
    // Timestamps - ¡USANDO time()!
    time_t now = time(0);
    nuevoPaciente.fechaCreacion = now;
    nuevoPaciente.fechaModificacion = now;
    
    // Abrir archivo en modo append
    ofstream archivo("pacientes.bin", ios::binary | ios::app);
    if (!archivo.is_open()) return false;
    
    archivo.write((char*)&nuevoPaciente, sizeof(Paciente));
    archivo.close();
    

    // Actualizando header
    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    actualizarHeaderPacientes(header);
    
    actualizarContadorHospital("paciente");
    
    return true;
}

// Función completa que crea y guarda el paciente 
bool registrarPacienteCompleto(const char* nombre, const char* apellido, 
                              const char* cedula, int edad, char sexo, const char* tipoSangre, 
                              const char* alergias, const char* telefono, const char* direccion, 
                              const char* email) {
    
    // 1. Crear el paciente en memoria
    Paciente nuevoPaciente = crearPaciente(nombre, apellido, cedula, edad, sexo, 
                                          tipoSangre, alergias, telefono, direccion, email);
    
    // 2. Guardar en archivo binario (esto asignará el ID automáticamente)
    bool resultado = agregarPaciente(nuevoPaciente);
    
    return resultado;
}

//---------------------------------------------
// CAPTURAR DATOS DE PACIENTE Y CREARLO
//---------------------------------------------
void capturarDatosPacienteVisual(Paciente* pacienteExistente = nullptr, bool soloLectura = false) {
    // Variables locales para capturar datos
    char nombre[50], apellido[50], cedula[20], tipoSangre[5];
    char alergias[500], telefono[15], direccion[100], email[50];
    int edad;
    char sexo;
    
    // Si se esta editando, cargar los datos existentes
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
        leerCampo(25, 13, 50, alergias, true);
        leerCampo(25, 16, 14, telefono, true);
        leerCampo(25, 17, 50, direccion, true);
        leerCampo(25, 18, 49, email, true);        
        
        
        
    } else {
        // Inicializar vacios para nuevo paciente
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
        titulo = esEdicion ? "MODIFICARa DATOS DEL PACIENTE" : "REGISTRO DE NUEVO PACIENTE";
    }
    textoColor(25 - titulo.length()/2, 3, titulo, 14, 1);
    
    // FORMULARIO DE CAPTURA
    textoColor(10, 4, "Datos Personales:", 14, 1);
    textoColor(10, 5, "Nombre:", 15, 1);
    textoColor(10, 6, "Apellido:", 15, 1);
    textoColor(10, 7, "Cedula:", 15, 1);
    textoColor(10, 8, "Edad:", 15, 1);
    textoColor(10, 9, "Sexo (M/F):", 15, 1);
    
    textoColor(10, 11, "Datos Medicos:", 14, 1);
    textoColor(10, 12, "Tipo de Sangre:", 15, 1);
    textoColor(10, 13, "Alergias:", 15, 1);
    
    textoColor(10, 15, "Contacto:", 14, 1);
    textoColor(10, 16, "Telefono:", 15, 1);
    textoColor(10, 17, "Direccion:", 15, 1);
    textoColor(10, 18, "Email:", 15, 1);
    
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
    
    // Cedula (solo validar duplicados si es nuevo paciente)
    do {
        temp = leerCampo(25, 7, 19, esEdicion ? cedula : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            if (!esEdicion) {
                // Solo verificar duplicados para nuevos pacientes
                Paciente existente = buscarPacientePorCedula(temp.c_str());
                if (existente.id != -1) {
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
    temp = leerCampo(25, 13, 50, esEdicion ? alergias : "", soloLectura);
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
        temp = leerCampo(25, 17, 50, esEdicion ? direccion : "", soloLectura);
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
            size_t posArroba = temp.find('@');
            if (posArroba != string::npos) {
                // Verificar que hay al menos un punto después del @
                size_t posPunto = temp.find('.', posArroba);
                if (posPunto != string::npos && posPunto > posArroba + 1 && posPunto < temp.length() - 1) {
                    strcpy(email, temp.c_str());
                    break;
                } else {
                    mostrarError(10, 20, "Error: Email valido (ej: usuario@dominio.com)!");
                }
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
            // Actualizar paciente existente
            strcpy(pacienteExistente->nombre, nombre);
            strcpy(pacienteExistente->apellido, apellido);
            strcpy(pacienteExistente->cedula, cedula);
            pacienteExistente->edad = edad;
            pacienteExistente->sexo = sexo;
            strcpy(pacienteExistente->tipoSangre, tipoSangre);
            strcpy(pacienteExistente->alergias, alergias);
            strcpy(pacienteExistente->telefono, telefono);
            strcpy(pacienteExistente->direccion, direccion);
            strcpy(pacienteExistente->email, email);
            pacienteExistente->fechaModificacion = time(0);
            
            // Guardar en archivo
            if (actualizarPaciente(*pacienteExistente)) {
                mostrarMensajeExito(10, 20,"Datos actualizados exitosamente!");
            } else {
                mostrarError(10, 20, "Error al actualizar paciente en archivo!");
            }
        } else {
            // Crear nuevo paciente usando la función que ya tenemos
            if (registrarPacienteCompleto(nombre, apellido, cedula, edad, sexo, tipoSangre, 
                                        alergias, telefono, direccion, email)) {
                mostrarMensajeExito(10, 20,"Paciente registrado exitosamente!");
            } else {
                mostrarError(10, 20, "Error al registrar paciente en archivo!");
            }
        }
    }
    
    if (soloLectura) {
        textoColor(10, 20, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
    }
}



//---------------------------------------------
// MOSTRAR PACIENTES EN TABLA 
//---------------------------------------------
void mostrarPacientesEnTabla(Paciente* pacientes, int cantidad, const string& titulo) {
   
   int startX = -5; int startY = 1;

    if (pacientes == nullptr || cantidad == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ERROR", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes para mostrar!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
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
        Paciente paciente = pacientes[i];  // Cambio: paciente por valor, no puntero
        
        // Solo mostrar pacientes activos y no eliminados
        if (!paciente.activo || paciente.eliminado) continue;
        
        // Alternar colores para mejor legibilidad
        int colorFila = (i % 2 == 0) ? 15 : 7;
        
        // Línea lateral izquierda
        textoColor(startX + 14, y, LINEA_VE, 14, 1);
        
        // ID
        textoColor(startX + 16, y, to_string(paciente.id), colorFila, 1);
        
        // Nombre completo
        string nombreCompleto = string(paciente.nombre) + " " + string(paciente.apellido);
        if (nombreCompleto.length() > 19) {
            nombreCompleto = nombreCompleto.substr(0, 16) + "...";
        }
        textoColor(startX + 22, y, nombreCompleto, colorFila, 1);
        
        // cedula
        textoColor(startX + 44, y, paciente.cedula, colorFila, 1);
        
        // Edad
        textoColor(startX + 59, y, to_string(paciente.edad), colorFila, 1);
        
        // Consultas
        textoColor(startX + 67, y, to_string(paciente.cantidadConsultas), colorFila, 1);
        
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



Paciente* leerTodosPacientesActivos(int* cantidadActivos = nullptr) {
	
    ifstream archivo("pacientes.bin", ios::binary);
    if (!archivo.is_open()) {
        if (cantidadActivos) *cantidadActivos = 0;
        return nullptr;
    }
    
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    if (header.cantidadRegistros == 0) {
        archivo.close();
        if (cantidadActivos) *cantidadActivos = 0;
        return nullptr;
    }
    
    // Contar cuántos están activos
    Paciente temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    int activos = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Paciente));
        if (!temp.eliminado && temp.activo) {
            activos++;
        }
    }
    
    if (activos == 0) {
        archivo.close();
        if (cantidadActivos) *cantidadActivos = 0;
        return nullptr;
    }
    
    Paciente* pacientesActivos = new Paciente[activos];
    
    // Volver al inicio y guardar los activos
    archivo.seekg(sizeof(ArchivoHeader));
    int indice = 0;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Paciente));
        if (!temp.eliminado && temp.activo) {
            pacientesActivos[indice] = temp;
            indice++;
        }
    }
    
    archivo.close();
    
    if (cantidadActivos) {
        *cantidadActivos = activos;
    }
    
    return pacientesActivos;
}



//---------------------------------------------
// LISTAR PACIENTES 
//---------------------------------------------
void listarPacientesVisual() {
    int cantidadActivos = 0;
    
    // Leer todos los pacientes activos del archivo (pasando cantidadActivos por referencia)
    Paciente* todosPacientes = leerTodosPacientesActivos(&cantidadActivos);
    
    if (todosPacientes == nullptr || cantidadActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "LISTA DE PACIENTES", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    mostrarPacientesEnTabla(todosPacientes, cantidadActivos, "LISTA DE PACIENTES");
    
    delete[] todosPacientes;
}



//---------------------------------------------
// BUSCAR Y MOSTRAR PACIENTE POR CEDULA
//---------------------------------------------
void buscarPacientePorCedulaVisual(bool soloLectura) {
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
    Paciente paciente = buscarPacientePorCedula(cedula.c_str());
    
    if (paciente.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, titulo, 14, 1);
        textoColor(10, 5, "Error: No se encontro ningun paciente con cedula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    if (!paciente.activo) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, titulo, 14, 1);
        textoColor(10, 5, "Error: El paciente esta inactivo en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Llamar a capturarDatosPacienteVisual con el modo correspondiente
    capturarDatosPacienteVisual(&paciente, soloLectura);
}


//---------------------------------------------
// BUSCAR Y MOSTRAR PACIENTES POR NOMBRE (INTERFAZ DE USUARIO)
//---------------------------------------------
void buscarPacientesPorNombreVisual() {
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
    
    // Buscar pacientes (USANDO LA NUEVA VERSIÓN PARA ARCHIVOS)
    int cantidadResultados = 0;
    Paciente* resultados = buscarPacientesPorNombre(nombreBuscado, &cantidadResultados);
    
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
    mostrarPacientesEnTabla(resultados, cantidadResultados, titulo);
    
    // Liberar memoria del array dinámico
    delete[] resultados;
}



//---------------------------------------------
// ELIMINAR PACIENTE POR ID
//---------------------------------------------
bool eliminarPaciente(int id) {
    // Buscar paciente por ID (incluyendo eliminados)
    Paciente paciente = buscarPacientePorID(id);
    if (paciente.id == -1) {
        return false; // Paciente no existe
    }
    
    // Verificar si ya está eliminado
    if (paciente.eliminado) {
        return false; // Ya estaba eliminado
    }
    
    // Marcar como eliminado (borrado lógico)
    paciente.eliminado = true;
    paciente.activo = false;
    paciente.fechaModificacion = time(0);
    
    // Guardar cambios en archivo
    if (!actualizarPaciente(paciente)) {
        return false; // Error al guardar
    }
    
    // Actualizar header (disminuir registros activos)
    ArchivoHeader header = leerHeaderPacientes();
    header.registrosActivos--;
    if (!actualizarHeaderPacientes(header)) {
        return false; // Error al actualizar header
    }
    
    return true;
}


//---------------------------------------------
// ELIMINAR PACIENTE POR CÉDULA (INTERFAZ DE USUARIO)
//---------------------------------------------
bool eliminarPacienteVisual() {
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
    
    // Buscar paciente por cedula (NUEVA VERSIÓN para archivos)
    Paciente paciente = buscarPacientePorCedula(cedula.c_str());
    
    if (paciente.id == -1) {
        system("cls");
        system("color 1F");        
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR PACIENTE", 14, 1);
        textoColor(10, 5, "Error: No se encontro ningun paciente con cedula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Verificar si ya está eliminado
    if (paciente.eliminado) {
        system("cls");
        system("color 1F");        
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR PACIENTE", 14, 1);
        textoColor(10, 5, "Error: El paciente ya fue eliminado anteriormente!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Mostrar confirmacion
    system("cls");
    system("color 1F");    
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "CONFIRMAR ELIMINACION", 14, 1);
    textoColor(10, 5, "¿Esta seguro que desea eliminar al siguiente paciente?", 15, 1);
    textoColor(10, 6, "ID: " + to_string(paciente.id), 15, 1);
    textoColor(10, 7, "Nombre: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
    textoColor(10, 8, "Cedula: " + string(paciente.cedula), 15, 1);
    textoColor(10, 9, "NOTA: Esta accion es un borrado logico (se puede recuperar)", 14, 1);
    textoColor(10, 10, "Presione S para confirmar, cualquier otra tecla para cancelar", 14, 1);
    
    char confirmacion = toupper(_getch());
    
    if (confirmacion == 'S') {
        // Eliminar paciente usando la función del Proyecto 2
        bool resultado = eliminarPaciente(paciente.id);
        
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR PACIENTE", 14, 1);
        
        if (resultado) {
            textoColor(10, 5, "? Paciente eliminado exitosamente!", 10, 1);
            textoColor(10, 6, "El paciente fue marcado como eliminado (borrado logico)", 15, 1);
        } else {
            textoColor(10, 5, "? Error: No se pudo eliminar el paciente!", 12, 1);
        }
        
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return resultado;
    } else {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR PACIENTE", 14, 1);
        textoColor(10, 5, "? Eliminacion cancelada por el usuario", 10, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
    }
    
    return false; 
}

//---------------------------------------------
// FUNCIONES DE DOCTORES 
//---------------------------------------------


// BUSCAR DOCTOR POR CÉDULA
//---------------------------------------------
Doctor buscarDoctorPorCedula(const char* cedula) {
	
    if (cedula == nullptr || strlen(cedula) == 0) {
        Doctor vacio;
        vacio.id = -1;
        return vacio;
    }
    
    // Abriendo archivo de doctores
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        Doctor vacio;
        vacio.id = -1;
        return vacio;
    }
    
    // Leyendo header para saber cantidad de registros
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    // Saltando header y buscar en todos los doctores
    archivo.seekg(sizeof(ArchivoHeader));
    
    Doctor temp;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        
        // Verificar que el doctor esté activo y comparar cédulas
        if (!temp.eliminado && temp.activo && compararStrings(temp.cedula, cedula)) {
            archivo.close();
            return temp; // Encontrado
        }
    }
    
    archivo.close();
    
    // No encontrado
    Doctor vacio;
    vacio.id = -1;
    return vacio;
}

//---------------------------------------------
// BUSCAR DOCTORES POR NOMBRE (PARCIAL, CASE-INSENSITIVE)
//---------------------------------------------
Doctor* buscarDoctoresPorNombre(const char* nombreBuscado, int* cantidadResultados) {
	
    *cantidadResultados = 0;
    
    if (nombreBuscado == nullptr || strlen(nombreBuscado) == 0) {
        return nullptr;
    }
    
    // Abrir archivo de doctores
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        return nullptr;
    }
    
    // Leer header para saber cuántos registros hay
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    if (header.cantidadRegistros == 0) {
        archivo.close();
        return nullptr;
    }
    
    // Contando cuántos doctores coinciden
    Doctor temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    int contador = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        
        if (!temp.eliminado && temp.activo) {
            // Crear nombre completo temporal
            char nombreCompleto[101]; // nombre(50) + espacio + apellido(50) = 101
            strcpy(nombreCompleto, temp.nombre);
            strcat(nombreCompleto, " ");
            strcat(nombreCompleto, temp.apellido);
            
            if (contieneSubstringCaseInsensitive(nombreCompleto, nombreBuscado)) {
                contador++;
            }
        }
    }
    
    if (contador == 0) {
        archivo.close();
        *cantidadResultados = 0;
        return nullptr;
    }
    
    Doctor* resultados = new Doctor[contador];
    
    // Volver al inicio del archivo
    archivo.seekg(sizeof(ArchivoHeader));
    int index = 0;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        
        if (!temp.eliminado && temp.activo) {
            // Crear nombre completo temporal
            char nombreCompleto[101];
            strcpy(nombreCompleto, temp.nombre);
            strcat(nombreCompleto, " ");
            strcat(nombreCompleto, temp.apellido);
            
            if (contieneSubstringCaseInsensitive(nombreCompleto, nombreBuscado)) {
                resultados[index++] = temp;
            }
        }
    }
    
    archivo.close();
    *cantidadResultados = contador;
    return resultados;
}

//---------------------------------------------
// BUSCAR DOCTORES POR ESPECIALIDAD (PARCIAL, CASE-INSENSITIVE)
//---------------------------------------------
Doctor* buscarDoctoresPorEspecialidad(const char* especialidadBuscada, int* cantidadResultados) {
	
    *cantidadResultados = 0;
    
    if (especialidadBuscada == nullptr || strlen(especialidadBuscada) == 0) {
        return nullptr;
    }
    
    // Abrir archivo de doctores
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        return nullptr;
    }
    
    // Leer header para saber cuántos registros hay
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    if (header.cantidadRegistros == 0) {
        archivo.close();
        return nullptr;
    }
    
    // Contando cuántos doctores coinciden
    Doctor temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    int contador = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        
        if (!temp.eliminado && temp.activo) {
            if (contieneSubstringCaseInsensitive(temp.especialidad, especialidadBuscada)) {
                contador++;
            }
        }
    }
    
    if (contador == 0) {
        archivo.close();
        *cantidadResultados = 0;
        return nullptr;
    }
    

    Doctor* resultados = new Doctor[contador];
    
    // Volver al inicio del archivo
    archivo.seekg(sizeof(ArchivoHeader));
    int index = 0;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        
        if (!temp.eliminado && temp.activo) {
            if (contieneSubstringCaseInsensitive(temp.especialidad, especialidadBuscada)) {
                resultados[index++] = temp;
            }
        }
    }
    
    archivo.close();
    *cantidadResultados = contador;
    return resultados;
}

//---------------------------------------------
// BUSCAR DOCTOR POR ID
//---------------------------------------------
Doctor buscarDoctorPorID(int id) {

    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        Doctor vacio;
        vacio.id = -1;
        return vacio;
    }
    
    // Leyendo header para saber cantidad de registros
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    // Saltando header: seekg(sizeof(ArchivoHeader))
    archivo.seekg(sizeof(ArchivoHeader));
    
    // Leyendo cada doctor hasta encontrar ID o EOF
    Doctor temp;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        if (temp.id == id && !temp.eliminado) {
            archivo.close();
            return temp; // Encontrado
        }
    }
    
    // 5. Retornar doctor vacío si no existe
    archivo.close();
    Doctor vacio;
    vacio.id = -1;
    return vacio;
}

//---------------------------------------------
// BUSCAR ÍNDICE DE DOCTOR POR ID
//---------------------------------------------
int buscarIndiceDoctorPorID(int id) {
	
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) return -1;
    
    // Leer header
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    // Buscar en todos los registros
    Doctor temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        if (temp.id == id && !temp.eliminado) {
            archivo.close();
            return i;  // Retorna el índice (posición en el archivo)
        }
    }
    
    archivo.close();
    return -1;  // No encontrado
}

//---------------------------------------------
// CALCULAR POSICIÓN EN ARCHIVO PARA DOCTORES
//---------------------------------------------
long calcularPosicionDoctor(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(Doctor));
}

//---------------------------------------------
// ACTUALIZAR DOCTOR EN ARCHIVO
//---------------------------------------------
bool actualizarDoctor(Doctor doctorModificado) {
    // Buscar la posición del doctor en el archivo
    int indice = buscarIndiceDoctorPorID(doctorModificado.id);
    if (indice == -1) return false;
    
    // Actualizar timestamp
    doctorModificado.fechaModificacion = time(0);
    
    // Abrir archivo y guardar cambios
    fstream archivo("doctores.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    
    long posicion = calcularPosicionDoctor(indice);
    archivo.seekp(posicion);
    archivo.write((char*)&doctorModificado, sizeof(Doctor));
    archivo.close();
    
    return true;
}

//---------------------------------------------
// CREAR DOCTOR (EN MEMORIA)
//---------------------------------------------
Doctor crearDoctor(const char* nombre, const char* apellido,
                  const char* cedula, const char* especialidad,
                  int aniosExperiencia, float costoConsulta,
                  const char* telefono = "0261-0000000",
                  const char* email = "doctor@hospital.com") {
    
    Doctor nuevoDoctor;
    
    // INICIALIZAR DOCTOR (sin ID todavía - se asignará al guardar)
    nuevoDoctor.activo = true;
    
    // Copiar datos básicos
    strncpy(nuevoDoctor.nombre, nombre, 49);
    strncpy(nuevoDoctor.apellido, apellido, 49);
    strncpy(nuevoDoctor.cedula, cedula, 19);
    strncpy(nuevoDoctor.especialidad, especialidad, 49);
    nuevoDoctor.aniosExperiencia = aniosExperiencia;
    nuevoDoctor.costoConsulta = costoConsulta;
    strncpy(nuevoDoctor.telefono, telefono, 14);
    strncpy(nuevoDoctor.email, email, 49);
    
    // Configuración adicional
    strcpy(nuevoDoctor.horarioAtencion, "Lun-Vie 8:00-16:00");
    
    // Inicializar arrays fijos y contadores
    nuevoDoctor.cantidadPacientes = 0;
    nuevoDoctor.cantidadCitas = 0;
    
    // Inicializar arrays de IDs
    for (int i = 0; i < 20; i++) {
        nuevoDoctor.pacientesIDs[i] = -1;
        nuevoDoctor.citasIDs[i] = -1;
    }
    
    // Metadata
    nuevoDoctor.eliminado = false;
    time_t now = time(0);
    nuevoDoctor.fechaCreacion = now;
    nuevoDoctor.fechaModificacion = now;
    
    return nuevoDoctor;
}

//---------------------------------------------
// INICIALIZAR ARCHIVO DE DOCTORES
//---------------------------------------------



//---------------------------------------------
// ACTUALIZAR HEADER DE DOCTORES
//---------------------------------------------
bool actualizarHeaderDoctores(ArchivoHeader nuevoHeader) {
	
    fstream archivo("doctores.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    
    archivo.seekp(0);
    archivo.write((char*)&nuevoHeader, sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

//---------------------------------------------
// AGREGAR DOCTOR AL ARCHIVO
//---------------------------------------------
bool agregarDoctor(Doctor& nuevoDoctor) {
	
    ArchivoHeader header = leerHeaderDoctores();
    
    // Configurando datos del doctor
    nuevoDoctor.id = header.proximoID;
    nuevoDoctor.activo = true;
    nuevoDoctor.eliminado = false;
    nuevoDoctor.cantidadPacientes = 0;
    nuevoDoctor.cantidadCitas = 0;
    
    // Inicializando arrays de IDs (Doctor tiene arrays de 50 y 30 según la estructura)
    for (int i = 0; i < 50; i++) {
        nuevoDoctor.pacientesIDs[i] = -1;
    }
    for (int i = 0; i < 30; i++) {
        nuevoDoctor.citasIDs[i] = -1;
    }
    
    // Timestamps
    time_t now = time(0);
    nuevoDoctor.fechaCreacion = now;
    nuevoDoctor.fechaModificacion = now;
    
    // Abriendo archivo en modo append
    ofstream archivo("doctores.bin", ios::binary | ios::app);
    if (!archivo.is_open()) return false;
    
    archivo.write((char*)&nuevoDoctor, sizeof(Doctor));
    archivo.close();
    
    // Actualizar header
    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    actualizarHeaderDoctores(header);
    
    // Actualizar contador del hospital
    actualizarContadorHospital("doctor");
    
    return true;
}

//---------------------------------------------
// REGISTRAR DOCTOR COMPLETO (FUNCIÓN PRINCIPAL)
//---------------------------------------------
bool registrarDoctorCompleto(const char* nombre, const char* apellido,
                           const char* cedula, const char* especialidad,
                           int aniosExperiencia, float costoConsulta,
                           const char* telefono, const char* email) {
    
    // Creando el doctor en memoria
    Doctor nuevoDoctor = crearDoctor(nombre, apellido, cedula, especialidad,
                                   aniosExperiencia, costoConsulta,
                                   telefono, email);
    
    // 2. Guardar en archivo binario (esto asignará el ID automáticamente)
    bool resultado = agregarDoctor(nuevoDoctor);
    
    return resultado;
}

//---------------------------------------------
// LEER TODOS LOS DOCTORES ACTIVOS
//---------------------------------------------
Doctor* leerTodosDoctoresActivos(int* cantidadActivos = nullptr) {
	
    ifstream archivo("doctores.bin", ios::binary);
    if (!archivo.is_open()) {
        if (cantidadActivos) *cantidadActivos = 0;
        return nullptr;
    }
    
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    if (header.cantidadRegistros == 0) {
        archivo.close();
        if (cantidadActivos) *cantidadActivos = 0;
        return nullptr;
    }
    
    // Contando cuántos están activos
    Doctor temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    int activos = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        if (!temp.eliminado && temp.activo) {
            activos++;
        }
    }
    
    if (activos == 0) {
        archivo.close();
        if (cantidadActivos) *cantidadActivos = 0;
        return nullptr;
    }
    
    // Crear array para doctores activos
    Doctor* doctoresActivos = new Doctor[activos];
    
    // Volver al inicio y guardar los activos
    archivo.seekg(sizeof(ArchivoHeader));
    int indice = 0;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        if (!temp.eliminado && temp.activo) {
            doctoresActivos[indice] = temp;
            indice++;
        }
    }
    
    archivo.close();
    
    if (cantidadActivos) {
        *cantidadActivos = activos;
    }
    
    return doctoresActivos;
}

//---------------------------------------------
// ELIMINAR DOCTOR POR ID (BORRADO LÓGICO)
//---------------------------------------------

bool eliminarDoctor(int id) {
    // Buscando doctor por ID
    Doctor doctor = buscarDoctorPorID(id);
    if (doctor.id == -1) {
        return false; // Doctor no encontrado
    }
    
    // Verificando si ya está eliminado
    if (doctor.eliminado) {
        return false; // Ya estaba eliminado
    }
    
    // Verificando si el doctor tiene pacientes asignados
    if (doctor.cantidadPacientes > 0) {
        return false; // No se puede eliminar si tiene pacientes asignados
    }
    
    // Verificando si el doctor tiene citas pendientes
    bool tieneCitasPendientes = false;
    for (int i = 0; i < doctor.cantidadCitas; i++) {
        if (doctor.citasIDs[i] != -1) {
            tieneCitasPendientes = true;
            break;
        }
    }
    
    if (tieneCitasPendientes) {
        for (int i = 0; i < doctor.cantidadCitas; i++) {
            if (doctor.citasIDs[i] != -1) {
             //cancelarCita(doctor.citasIDs[i]);
            }
        }
    }
    
    // Marcando como eliminado (borrado lógico)
    doctor.eliminado = true;
    doctor.activo = false;
    doctor.fechaModificacion = time(0);
    
    // Limpiando arrays de IDs
    for (int i = 0; i < 20; i++) {
        doctor.pacientesIDs[i] = -1;
        doctor.citasIDs[i] = -1;
    }
    doctor.cantidadPacientes = 0;
    doctor.cantidadCitas = 0;
    
    // Guardando cambios en archivo
    if (!actualizarDoctor(doctor)) {
        return false; // Error al guardar
    }
    
    // Actualizndo header 
    ArchivoHeader header = leerHeaderDoctores();
    header.registrosActivos--;
    if (!actualizarHeaderDoctores(header)) {
        return false; // Error al actualizar header
    }
    
    return true;
}



//---------------------------------------------
// ASIGNAR PACIENTE A DOCTOR (FUNCIÓN PRINCIPAL PARA ARCHIVOS)
//---------------------------------------------
bool asignarPacienteADoctorArchivos(int idPaciente, int idDoctor) {
	
    // Obteniendo paciente y doctor actualizados
    Paciente paciente = buscarPacientePorID(idPaciente);
    Doctor doctor = buscarDoctorPorID(idDoctor);
    
    if (paciente.id == -1 || doctor.id == -1) {
        return false;
    }
    
    // Verificando que ambos estén activos
    if (!paciente.activo || paciente.eliminado || !doctor.activo || doctor.eliminado) {
        return false;
    }
    
    // Verificando si ya están asignados
    for (int i = 0; i < doctor.cantidadPacientes; i++) {
        if (doctor.pacientesIDs[i] == idPaciente) {
            return false; // Ya asignado
        }
    }
    
    // Verificando si hay espacio en el array del doctor
    if (doctor.cantidadPacientes >= 20) { // Tamaño fijo del array
        return false; // No hay espacio
    }
    
    // Verificando si hay espacio en el array del paciente
    if (paciente.cantidadCitas >= 20) { // Tamaño fijo del array
        return false; // No hay espacio
    }
    
    // Verificando paciente al doctor
    doctor.pacientesIDs[doctor.cantidadPacientes] = idPaciente;
    doctor.cantidadPacientes++;
    doctor.fechaModificacion = time(0);
    
    
    // Guardando cambios en archivos
    if (!actualizarDoctor(doctor)) {
        return false;
    }
    

    return true;
}


//---------------------------------------------
// CAPTURAR DATOS DE DOCTOR Y CREARLO
//---------------------------------------------
void capturarDatosDoctorVisual(Doctor* doctorExistente = nullptr, bool soloLectura = false) {
	
    // Variables locales para capturar datos
    char nombre[50], apellido[50], cedula[20], especialidad[50];
    char telefono[15], email[50];
    int aniosExperiencia;
    float costoConsulta;
    
    // Si estamos editando, cargar los datos existentes
    bool esEdicion = (doctorExistente != nullptr);

    system("cls");
    system("color 1F");

    if (esEdicion) {
        strcpy(nombre, doctorExistente->nombre);
        strcpy(apellido, doctorExistente->apellido);
        strcpy(cedula, doctorExistente->cedula);
        strcpy(especialidad, doctorExistente->especialidad);
        strcpy(telefono, doctorExistente->telefono);
        strcpy(email, doctorExistente->email);
        aniosExperiencia = doctorExistente->aniosExperiencia;
        costoConsulta = doctorExistente->costoConsulta;
        leerCampo(25, 5, 49, nombre, true);        
        leerCampo(25, 6, 49, apellido, true);        
        leerCampo(25, 7, 19, cedula, true); 
        leerCampo(25, 10, 49, especialidad, true);
        leerCampo(28, 11, 2, to_string(aniosExperiencia), true);
        leerCampo(26, 12, 6, to_string(costoConsulta), true);
        leerCampo(25, 15, 14, telefono, true);
        leerCampo(25, 16, 49, email, true);        
        
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
    
    system("color 1F");
    dibujarCuadro(2, 3, 75, 20);
    
    string titulo;
    if (soloLectura) {
        titulo = "INFORMACIÓN DEL DOCTOR";
    } else {
        titulo = esEdicion ? "MODIFICAR DATOS DEL DOCTOR" : "REGISTRO DE NUEVO DOCTOR";
    }
    textoColor(25 - titulo.length()/2, 3, titulo, 14, 1);
    
    // FORMULARIO DE CAPTURA
    textoColor(10, 4, "Datos Personales:", 14, 1);
    textoColor(10, 5, "Nombre:", 15, 1);
    textoColor(10, 6, "Apellido:", 15, 1);
    textoColor(10, 7, "Cédula:", 15, 1);
    
    textoColor(10, 9, "Datos Profesionales:", 14, 1);
    textoColor(10, 10, "Especialidad:", 15, 1);
    textoColor(10, 11, "Años Experiencia:", 15, 1);
    textoColor(10, 12, "Costo Consulta:", 15, 1);
    
    textoColor(10, 14, "Contacto:", 14, 1);
    textoColor(10, 15, "Teléfono:", 15, 1);
    textoColor(10, 16, "Email:", 15, 1);
    
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
        mostrarError(10, 18, "Error: El nombre no puede estar vacío!");
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
        mostrarError(10, 18, "Error: El apellido no puede estar vacío!");
    } while (true);
    
    // Cédula (solo validar duplicados si es nuevo doctor)
    do {
        temp = leerCampo(25, 7, 19, esEdicion ? cedula : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            if (!esEdicion) {
                // Solo verificar duplicados para nuevos doctores
                Doctor existente = buscarDoctorPorCedula(temp.c_str());
                if (existente.id != -1) {
                    mostrarError(10, 18, "Error: Ya existe un doctor con esta cédula!");
                    continue;
                }
            }
            strcpy(cedula, temp.c_str());
            break;
        }
        mostrarError(10, 18, "Error: La cédula no puede estar vacía!");
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
        mostrarError(10, 18, "Error: La especialidad no puede estar vacía!");
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
        mostrarError(10, 18, "Error: El teléfono no puede estar vacío!");
    } while (true);
    
    // Email
    do {
        temp = leerCampo(25, 16, 49, esEdicion ? email : "", soloLectura);
        if (temp == "ESC" && !soloLectura) return;
        if (soloLectura) break;
        
        if (!temp.empty()) {
            size_t posArroba = temp.find('@');
            if (posArroba != string::npos) {
                // Verificar que hay al menos un punto después del @
                size_t posPunto = temp.find('.', posArroba);
                if (posPunto != string::npos && posPunto > posArroba + 1 && posPunto < temp.length() - 1) {
                    strcpy(email, temp.c_str());
                    break;
                } else {
                    mostrarError(10, 18, "Error: Email válido (ej: usuario@dominio.com)!");
                }
            } else {
                mostrarError(10, 18, "Error: Email debe contener @!");
            }
        } else {
            mostrarError(10, 18, "Error: El email no puede estar vacío!");
        }
    } while (true);
    
    // CREAR O ACTUALIZAR DOCTOR (solo si no es solo lectura)
    if (!soloLectura) {
        if (esEdicion) {
            // Actualizar doctor existente
            strcpy(doctorExistente->nombre, nombre);
            strcpy(doctorExistente->apellido, apellido);
            strcpy(doctorExistente->cedula, cedula);
            strcpy(doctorExistente->especialidad, especialidad);
            doctorExistente->aniosExperiencia = aniosExperiencia;
            doctorExistente->costoConsulta = costoConsulta;
            strcpy(doctorExistente->telefono, telefono);
            strcpy(doctorExistente->email, email);
            doctorExistente->fechaModificacion = time(0);
            
            // Guardar en archivo
            if (actualizarDoctor(*doctorExistente)) {
                mostrarMensajeExito(10, 18, "Datos actualizados exitosamente!");
            } else {
                mostrarError(10, 18, "Error al actualizar doctor en archivo!");
            }
        } else {
            // Crear nuevo doctor usando la función que ya tenemos
            if (registrarDoctorCompleto(nombre, apellido, cedula, especialidad, 
                                      aniosExperiencia, costoConsulta, 
                                      telefono, email)) {
                mostrarMensajeExito(10, 18, "Doctor registrado exitosamente!");
            } else {
                mostrarError(10, 18, "Error al registrar doctor en archivo!");
            }
        }
    }
    
    if (soloLectura) {
        textoColor(10, 18, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
    }
}

//---------------------------------------------
// BUSCAR Y MOSTRAR DOCTOR POR ID (INTERFAZ DE USUARIO - VERSIÓN PARA ARCHIVOS)
//---------------------------------------------
void buscarDoctorPorIdVisual() {
    // Verificar si hay doctores en el sistema
    ArchivoHeader header = leerHeaderDoctores();
    if (header.cantidadRegistros == 0) {
        system("cls");
        system("color 1F");
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
        system("color 1F");
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
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTOR POR ID", 14, 1);
        textoColor(10, 5, "Error: El ID debe ser un numero positivo!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Buscar doctor por ID (VERSIÓN PARA ARCHIVOS)
    Doctor doctor = buscarDoctorPorID(idBuscado);
    
    if (doctor.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTOR POR ID", 14, 1);
        textoColor(10, 5, "Error: No se encontro ningun doctor con ID: " + to_string(idBuscado), 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Verificar si está inactivo o eliminado
    if (!doctor.activo || doctor.eliminado) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTOR POR ID", 14, 1);
        textoColor(10, 5, "Error: El doctor está inactivo o fue eliminado!", 12, 1);
        textoColor(10, 6, "ID: " + to_string(doctor.id) + " - " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Llamar a capturarDatosDoctorVisual en modo SOLO LECTURA
    capturarDatosDoctorVisual(&doctor, true); // true = solo lectura
}

//---------------------------------------------
// MOSTRAR DOCTORES EN TABLA (VERSIÓN CORREGIDA - RECIBE Doctor*)
//---------------------------------------------
void mostrarDoctoresEnTabla(Doctor* doctores, int cantidad, const string& titulo) {
 
    int startX = -6; 
    int startY = 0;

    if (doctores == nullptr || cantidad == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ERROR", 14, 1);
        textoColor(10, 5, "Error: No hay doctores para mostrar!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    
    int y = startY;
    
    // CABECERA DE LA TABLA CON TÍTULO DINÁMICO
    textoColor(startX + 14, y++, ES_SU_IZ + replicar(LINEA_HO[0], 59) + ES_SU_DE, 14, 1);
    textoColor(startX + 14, y++, LINEA_VE + "                    " + titulo + "                      " + LINEA_VE, 14, 1);
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 21) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_SUP + replicar(LINEA_HO[0], 17) + CRUZ_DER, 14, 1);
    
    // ENCABEZADO DE COLUMNAS
    textoColor(startX + 16, y, "Id", 14, 1);
    textoColor(startX + 22, y, "Nombre Completo", 14, 1);
    textoColor(startX + 44, y, "Cédula", 14, 1);
    textoColor(startX + 57, y, "Especialidad", 14, 1);

    // Líneas verticales del encabezado
    textoColor(startX + 14, y, LINEA_VE, 14, 1);
    textoColor(startX + 19, y, LINEA_VE, 14, 1);
    textoColor(startX + 41, y, LINEA_VE, 14, 1);
    textoColor(startX + 56, y, LINEA_VE, 14, 1);
    textoColor(startX + 74, y, LINEA_VE, 14, 1);
    y++;
    
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 21) + CRUZ_CEN + replicar(LINEA_HO[0], 14) + CRUZ_CEN + replicar(LINEA_HO[0], 17) + CRUZ_DER, 14, 1);
    
    // LISTA DE DOCTORES
    for (int i = 0; i < cantidad; i++) {
        Doctor& doctor = doctores[i];  // Ahora usa referencia en lugar de puntero
        
        // Alternar colores para mejor legibilidad
        int colorFila = (i % 2 == 0) ? 15 : 7;
        
        // Línea lateral izquierda
        textoColor(startX + 14, y, LINEA_VE, 14, 1);
        
        // ID
        textoColor(startX + 16, y, to_string(doctor.id), colorFila, 1);
        
        // Nombre completo
        string nombreCompleto = "Dr. " + string(doctor.nombre) + " " + string(doctor.apellido);
        if (nombreCompleto.length() > 19) {
            nombreCompleto = nombreCompleto.substr(0, 16) + "...";
        }
        textoColor(startX + 22, y, nombreCompleto, colorFila, 1);
        
        // Cédula
        textoColor(startX + 44, y, doctor.cedula, colorFila, 1);
        
        // Especialidad
        string especialidad = doctor.especialidad;
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
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 21) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_INF + replicar(LINEA_HO[0], 17) + CRUZ_DER, 14, 1);
            textoColor(startX + 19, y++, "Presione cualquier tecla para ver más...", 14, 1);
            _getch();
            
            // Nueva página
            system("cls");
            system("color 1F");
            y = startY;
            textoColor(startX + 14, y++, ES_SU_IZ + replicar(LINEA_HO[0], 59) + ES_SU_DE, 14, 1);
            textoColor(startX + 14, y++, LINEA_VE + "                    " + titulo + " (Cont.)              " + LINEA_VE, 14, 1);
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 21) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_SUP + replicar(LINEA_HO[0], 17) + CRUZ_DER, 14, 1);
            
            // Re-dibujar encabezado
            textoColor(startX + 16, y, "Id", 14, 1);
            textoColor(startX + 22, y, "Nombre Completo", 14, 1);
            textoColor(startX + 44, y, "Cédula", 14, 1);
            textoColor(startX + 57, y, "Especialidad", 14, 1);
            
            textoColor(startX + 14, y, LINEA_VE, 14, 1);
            textoColor(startX + 19, y, LINEA_VE, 14, 1);
            textoColor(startX + 41, y, LINEA_VE, 14, 1);
            textoColor(startX + 56, y, LINEA_VE, 14, 1);
            textoColor(startX + 74, y, LINEA_VE, 14, 1);
            y++;
            
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 21) + CRUZ_CEN + replicar(LINEA_HO[0], 14) + CRUZ_CEN + replicar(LINEA_HO[0], 17) + CRUZ_DER, 14, 1);
        }
    }
    
    // PIE DE TABLA
    textoColor(startX + 14, y++, ES_IN_IZ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 21) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_INF + replicar(LINEA_HO[0], 17) + ES_IN_DE, 14, 1);
    textoColor(startX + 15, y++, "Total encontrados: " + to_string(cantidad), 14, 1);
    textoColor(startX + 15, y++, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}

//---------------------------------------------
// BUSCAR Y MOSTRAR DOCTORES POR ESPECIALIDAD
//---------------------------------------------
void buscarDoctoresPorEspecialidadVisual() {
    // Verificar si hay doctores en el sistema
    ArchivoHeader header = leerHeaderDoctores();
    if (header.cantidadRegistros == 0) {
        system("cls");
        system("color 1F");
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
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTORES POR ESPECIALIDAD", 14, 1);
        textoColor(10, 5, "Error: La especialidad no puede estar vacía!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    strcpy(especialidadBuscada, temp.c_str());
    
    // Buscar doctores
    int cantidadResultados = 0;
    Doctor* resultados = buscarDoctoresPorEspecialidad(especialidadBuscada, &cantidadResultados);
    
    if (resultados == nullptr || cantidadResultados == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "BUSCAR DOCTORES POR ESPECIALIDAD", 14, 1);
        textoColor(10, 5, "No se encontraron doctores con especialidad: '" + string(especialidadBuscada) + "'", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Mostrar resultados en tabla usando la versión corregida
    string titulo = "DOCTORES DE: " + string(especialidadBuscada);
    mostrarDoctoresEnTabla(resultados, cantidadResultados, titulo);
    
    // Liberar memoria (solo un array ahora)
    delete[] resultados;
}

//---------------------------------------------
// ASIGNAR PACIENTE A DOCTOR (VERSIÓN PARA ARCHIVOS)
//---------------------------------------------
void asignarPacienteADoctorVisual() {
    // Verificar si hay pacientes y doctores en el sistema
    ArchivoHeader headerPacientes = leerHeaderPacientes();
    ArchivoHeader headerDoctores = leerHeaderDoctores();
    
    if (headerPacientes.registrosActivos == 0 || headerDoctores.registrosActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ASIGNAR PACIENTE A DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes o doctores registrados!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 12);
    
    textoColor(25, 3, "ASIGNAR PACIENTE A DOCTOR", 14, 1);
    textoColor(10, 5, "Cédula del Paciente: ", 15, 1);
    textoColor(10, 6, "Cédula del Doctor: ", 15, 1);
    textoColor(10, 8, "Nota: Un paciente puede tener múltiples doctores", 8, 1);
    textoColor(10, 9, "y un doctor puede tener múltiples pacientes.", 8, 1);
    textoColor(10, 11, "Presione ESC para cancelar", 14, 1);
    
    // Variables para captura
    char cedulaPaciente[20], cedulaDoctor[20];
    memset(cedulaPaciente, 0, sizeof(cedulaPaciente));
    memset(cedulaDoctor, 0, sizeof(cedulaDoctor));
    
    string temp;
    Paciente paciente;
    Doctor doctor;
    
    // Cédula del Paciente
    do {
        temp = leerCampo(32, 5, 19);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            paciente = buscarPacientePorCedula(temp.c_str());
            if (paciente.id != -1 && paciente.activo && !paciente.eliminado) {
                strcpy(cedulaPaciente, temp.c_str());
                break;
            } else {
                mostrarError(10, 12, "Error: No se encontró paciente activo con esa cédula!");
            }
        } else {
            mostrarError(10, 12, "Error: La cédula del paciente no puede estar vacía!");
        }
    } while (true);
    
    // Cédula del Doctor
    do {
        temp = leerCampo(32, 6, 19);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            doctor = buscarDoctorPorCedula(temp.c_str());
            if (doctor.id != -1 && doctor.activo && !doctor.eliminado) {
                strcpy(cedulaDoctor, temp.c_str());
                break;
            } else {
                mostrarError(10, 12, "Error: No se encontró doctor activo con esa cédula!");
            }
        } else {
            mostrarError(10, 12, "Error: La cédula del doctor no puede estar vacía!");
        }
    } while (true);
    
    // Verificar si ya están asignados
    bool yaAsignado = false;
    for (int i = 0; i < doctor.cantidadPacientes; i++) {
        if (doctor.pacientesIDs[i] == paciente.id) {
            yaAsignado = true;
            break;
        }
    }
    
    if (yaAsignado) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "ASIGNAR PACIENTE A DOCTOR", 14, 1);
        textoColor(10, 5, "Error: El paciente ya está asignado a este doctor!", 12, 1);
        textoColor(10, 6, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
        textoColor(10, 7, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
        textoColor(10, 8, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // MOSTRAR CONFIRMACIÓN
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "CONFIRMAR ASIGNACIÓN", 14, 1);
    textoColor(10, 5, "¿Confirmar asignación del siguiente paciente al doctor?", 15, 1);
    textoColor(10, 6, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
    textoColor(10, 7, "Cédula: " + string(paciente.cedula), 15, 1);
    textoColor(10, 8, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
    textoColor(10, 9, "Especialidad: " + string(doctor.especialidad), 15, 1);
    textoColor(10, 10, "Presione S para confirmar, cualquier otra tecla para cancelar", 14, 1);
    
    char confirmacion = _getch();
    
    if (confirmacion == 'S' || confirmacion == 's') {
        // Realizar la asignación
        bool resultado = asignarPacienteADoctorArchivos(paciente.id, doctor.id);
        
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "ASIGNAR PACIENTE A DOCTOR", 14, 1);
        
        if (resultado) {
            textoColor(10, 5, "¡Asignación exitosa!", 10, 1);
            textoColor(10, 6, "Paciente asignado correctamente al doctor.", 10, 1);
            textoColor(10, 7, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
            textoColor(10, 8, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
        } else {
            textoColor(10, 5, "Error: No se pudo realizar la asignación!", 12, 1);
        }
        
        textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
    }
}
//---------------------------------------------
// MOSTRAR PACIENTES ASIGNADOS A DOCTOR EN TABLA 
//---------------------------------------------
void mostrarPacientesDeDoctorEnTabla(Doctor* doctor, const string& titulo) {
    
    int startX = -6; 
    int startY = 0;

    if (doctor == nullptr || doctor->cantidadPacientes == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "PACIENTES ASIGNADOS", 14, 1);
        textoColor(10, 5, "El doctor no tiene pacientes asignados.", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    
    int y = startY;
    
    // CABECERA DE LA TABLA CON TÍTULO DINÁMICO
    textoColor(startX + 14, y++, ES_SU_IZ + replicar(LINEA_HO[0], 59) + ES_SU_DE, 14, 1);
    textoColor(startX + 14, y++, LINEA_VE + "                    " + titulo + "                       " + LINEA_VE, 14, 1);
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 21) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_SUP + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
    
    // ENCABEZADO DE COLUMNAS
    textoColor(startX + 16, y, "Id", 14, 1);
    textoColor(startX + 22, y, "Nombre Completo", 14, 1);
    textoColor(startX + 44, y, "Cédula", 14, 1);
    textoColor(startX + 57, y, "Edad", 14, 1);
    textoColor(startX + 64, y, "Sexo", 14, 1);

    // Líneas verticales del encabezado
    textoColor(startX + 14, y, LINEA_VE, 14, 1);
    textoColor(startX + 19, y, LINEA_VE, 14, 1);
    textoColor(startX + 41, y, LINEA_VE, 14, 1);
    textoColor(startX + 56, y, LINEA_VE, 14, 1);
    textoColor(startX + 63, y, LINEA_VE, 14, 1);
    textoColor(startX + 74, y, LINEA_VE, 14, 1);
    y++;
    
    textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 21) + CRUZ_CEN + replicar(LINEA_HO[0], 14) + CRUZ_CEN + replicar(LINEA_HO[0], 6) + CRUZ_CEN + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
    
    // LISTA DE PACIENTES ASIGNADOS
    int pacientesMostrados = 0;
    for (int i = 0; i < doctor->cantidadPacientes; i++) {
        int idPaciente = doctor->pacientesIDs[i];
        
        // Buscar paciente en archivo
        Paciente paciente = buscarPacientePorID(idPaciente);
        
        // Verificar si el paciente existe y está activo
        if (paciente.id == -1 || !paciente.activo || paciente.eliminado) {
            continue; // Saltar pacientes no encontrados o inactivos
        }
        
        // Alternar colores para mejor legibilidad
        int colorFila = (pacientesMostrados % 2 == 0) ? 15 : 7;
        
        // Línea lateral izquierda
        textoColor(startX + 14, y, LINEA_VE, 14, 1);
        
        // ID
        textoColor(startX + 16, y, to_string(paciente.id), colorFila, 1);
        
        // Nombre completo
        string nombreCompleto = string(paciente.nombre) + " " + string(paciente.apellido);
        if (nombreCompleto.length() > 19) {
            nombreCompleto = nombreCompleto.substr(0, 16) + "...";
        }
        textoColor(startX + 22, y, nombreCompleto, colorFila, 1);
        
        // Cédula
        textoColor(startX + 44, y, paciente.cedula, colorFila, 1);
        
        // Edad
        textoColor(startX + 59, y, to_string(paciente.edad), colorFila, 1);
        
        // Sexo
        textoColor(startX + 67, y, string(1, paciente.sexo), colorFila, 1);
        
        // Líneas verticales
        textoColor(startX + 19, y, LINEA_VE, 14, 1);
        textoColor(startX + 41, y, LINEA_VE, 14, 1);
        textoColor(startX + 56, y, LINEA_VE, 14, 1);
        textoColor(startX + 63, y, LINEA_VE, 14, 1);
        textoColor(startX + 74, y, LINEA_VE, 14, 1);
        
        y++;
        pacientesMostrados++;
        
        // Paginación (cada 10 pacientes)
        if (y >= (startY + 15) && i < doctor->cantidadPacientes - 1) {
            textoColor(startX + 14, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 21) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_INF + replicar(LINEA_HO[0], 6) + CRUZ_INF + replicar(LINEA_HO[0], 10) + CRUZ_DER, 14, 1);
            textoColor(startX + 19, y++, "Presione cualquier tecla para ver más...", 14, 1);
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
            textoColor(startX + 44, y, "Cédula", 14, 1);
            textoColor(startX + 57, y, "Edad", 14, 1);
            textoColor(startX + 64, y, "Sexo", 14, 1);
            
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
    textoColor(startX + 15, y++, "Total pacientes asignados: " + to_string(pacientesMostrados), 14, 1);
    textoColor(startX + 15, y++, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}
//---------------------------------------------
// MOSTRAR PACIENTES ASIGNADOS A DOCTOR (INTERFAZ DE USUARIO)
//---------------------------------------------
//---------------------------------------------
// MOSTRAR PACIENTES ASIGNADOS A DOCTOR (INTERFAZ VISUAL - VERSIÓN PARA ARCHIVOS)
//---------------------------------------------
void listarPacientesDeDoctorVisual() {
    // Verificar si hay doctores en el sistema
    ArchivoHeader headerDoctores = leerHeaderDoctores();
    ArchivoHeader headerPacientes = leerHeaderPacientes();
    
    if (headerDoctores.registrosActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "PACIENTES ASIGNADOS A DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No hay doctores registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    
    textoColor(25, 3, "PACIENTES ASIGNADOS A DOCTOR", 14, 1);
    textoColor(10, 5, "Ingrese la cédula del doctor: ", 15, 1);
    textoColor(10, 7, "Presione ESC para cancelar", 14, 1);
    
    // Leer cédula del doctor
    string cedula = leerCampo(42, 5, 19);
    
    if (cedula == "ESC") {
        return; 
    }
    
    if (cedula.empty()) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "PACIENTES ASIGNADOS A DOCTOR", 14, 1);
        textoColor(10, 5, "Error: La cédula no puede estar vacía!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Buscar doctor por cédula (VERSIÓN PARA ARCHIVOS)
    Doctor doctor = buscarDoctorPorCedula(cedula.c_str());
    
    if (doctor.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "PACIENTES ASIGNADOS A DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No se encontró doctor con cédula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Verificar si el doctor está activo
    if (!doctor.activo || doctor.eliminado) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "PACIENTES ASIGNADOS A DOCTOR", 14, 1);
        textoColor(10, 5, "Error: El doctor está inactivo o fue eliminado!", 12, 1);
        textoColor(10, 6, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    string titulo = "PACIENTES DE DR: " + string(doctor.nombre) + " " + string(doctor.apellido);
    mostrarPacientesDeDoctorEnTabla(&doctor, titulo);
}


//---------------------------------------------
// LISTAR DOCTORES 
//---------------------------------------------
void listarDoctoresVisual() {
    int cantidadActivos = 0;
    
    // Leer todos los doctores activos del archivo
    Doctor* todosDoctores = leerTodosDoctoresActivos(&cantidadActivos);
    
    if (todosDoctores == nullptr || cantidadActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "LISTA DE DOCTORES", 14, 1);
        textoColor(10, 5, "Error: No hay doctores registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Usar la versión corregida que recibe Doctor*
    mostrarDoctoresEnTabla(todosDoctores, cantidadActivos, "LISTA DE DOCTORES");
    
    // Liberar memoria (solo un array ahora)
    delete[] todosDoctores;
}


//---------------------------------------------
// ELIMINAR DOCTOR POR CEDULA 
//---------------------------------------------

bool eliminarDoctorVisual() {
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 10);
    
    textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
    textoColor(10, 5, "Ingrese la cédula del doctor a eliminar: ", 15, 1);
    textoColor(10, 7, "ADVERTENCIA: Esta acción es un borrado lógico!", 12, 1);
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
        textoColor(10, 5, "Error: La cédula no puede estar vacía!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Buscar doctor por cédula (VERSIÓN PARA ARCHIVOS)
    Doctor doctor = buscarDoctorPorCedula(cedula.c_str());
    
    if (doctor.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No se encontró ningún doctor con cédula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Verificar si ya está eliminado
    if (doctor.eliminado) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        textoColor(10, 5, "Error: El doctor ya fue eliminado anteriormente!", 12, 1);
        textoColor(10, 6, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Verificar si el doctor tiene pacientes asignados
    if (doctor.cantidadPacientes > 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No se puede eliminar el doctor porque tiene pacientes asignados!", 12, 1);
        textoColor(10, 6, "Pacientes asignados: " + to_string(doctor.cantidadPacientes), 15, 1);
        textoColor(10, 7, "Debe reasignar los pacientes a otro doctor primero.", 15, 1);
        textoColor(10, 8, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return false;
    }
    
    // Verificar si el doctor tiene citas pendientes
    bool tieneCitasPendientes = false;
    for (int i = 0; i < doctor.cantidadCitas; i++) {
        if (doctor.citasIDs[i] != -1) {
            // Nota: Aquí deberías verificar el estado de cada cita
            // Por simplicidad, asumimos que todas son pendientes
            tieneCitasPendientes = true;
            break;
        }
    }
    
    // Mostrar confirmación
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "CONFIRMAR ELIMINACIÓN", 14, 1);
    textoColor(10, 5, "¿Está seguro que desea eliminar al siguiente doctor?", 15, 1);
    textoColor(10, 6, "ID: " + to_string(doctor.id), 15, 1);
    textoColor(10, 7, "Nombre: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
    textoColor(10, 8, "Cédula: " + string(doctor.cedula), 15, 1);
    textoColor(10, 9, "Especialidad: " + string(doctor.especialidad), 15, 1);
    
    if (tieneCitasPendientes) {
        textoColor(10, 10, "ADVERTENCIA: El doctor tiene citas que serán canceladas!", 12, 1);
        textoColor(10, 11, "Presione S para confirmar, cualquier otra tecla para cancelar", 14, 1);
    } else {
        textoColor(10, 10, "El doctor no tiene citas pendientes.", 10, 1);
        textoColor(10, 11, "Presione S para confirmar, cualquier otra tecla para cancelar", 14, 1);
    }
    
    char confirmacion = _getch();
    
    if (confirmacion == 'S' || confirmacion == 's') {
        // Eliminar doctor usando la función del sistema de archivos
        bool resultado = eliminarDoctor(doctor.id);
        
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        
        if (resultado) {
            textoColor(10, 5, "? Doctor eliminado exitosamente!", 10, 1);
            textoColor(10, 6, "El doctor fue marcado como eliminado (borrado lógico)", 15, 1);
        } else {
            textoColor(10, 5, "? Error: No se pudo eliminar el doctor!", 12, 1);
        }
        
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return resultado;
    } else {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ELIMINAR DOCTOR", 14, 1);
        textoColor(10, 5, "? Eliminación cancelada por el usuario", 10, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
    }
    
    return false; 
}

//---------------------------------------------
// FUNCIONES DE CITAS 
//---------------------------------------------

// Actualizar header de citas
bool actualizarHeaderCitas(ArchivoHeader nuevoHeader) {
    fstream archivo("citas.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    
    archivo.seekp(0);
    archivo.write((char*)&nuevoHeader, sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

// Buscar cita por ID
Cita buscarCitaPorID(int id) {
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        Cita vacio;
        vacio.id = -1;
        return vacio;
    }
    
    // Leer header
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    if (header.cantidadRegistros == 0) {
        archivo.close();
        Cita vacio;
        vacio.id = -1;
        return vacio;
    }
    
    // Buscar la cita
    Cita temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Cita));
        if (temp.id == id && !temp.eliminada) {
            archivo.close();
            return temp;
        }
    }
    
    archivo.close();
    Cita vacio;
    vacio.id = -1;
    return vacio;
}

// Buscar índice de cita por ID
int buscarIndiceCitaPorID(int id) {
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) return -1;
    
    // Leer header
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    if (header.cantidadRegistros == 0) {
        archivo.close();
        return -1;
    }
    
    // Buscar la cita
    Cita temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Cita));
        if (temp.id == id && !temp.eliminada) {
            archivo.close();
            return i;
        }
    }
    
    archivo.close();
    return -1;
}

// Calcular posición en archivo para citas
long calcularPosicionCita(int indice) {
    return sizeof(ArchivoHeader) + (indice * sizeof(Cita));
}

// Actualizar cita en archivo
bool actualizarCita(Cita citaModificada) {
    // Buscar la posición de la cita en el archivo
    int indice = buscarIndiceCitaPorID(citaModificada.id);
    if (indice == -1) return false;
    
    // Actualizar timestamp de modificación
    citaModificada.fechaModificacion = time(0);
    
    // Abrir archivo y guardar cambios
    fstream archivo("citas.bin", ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    
    long posicion = calcularPosicionCita(indice);
    archivo.seekp(posicion);
    archivo.write((char*)&citaModificada, sizeof(Cita));
    archivo.close();
    
    return true;
}

bool agregarCita(Cita& nuevaCita) {
    ArchivoHeader header = leerHeaderCitas();
    
    // Configurar datos de la cita - ¡CORRECCIÓN IMPORTANTE!
    nuevaCita.id = header.proximoID;  // ID de la cita
    nuevaCita.atendida = false;
    nuevaCita.eliminada = false;
    nuevaCita.idHistorialAsociado = -1;  // Sin historial aún (no id)
    
    // Timestamps
    time_t now = time(0);
    nuevaCita.fechaCreacion = now;
    nuevaCita.fechaModificacion = now;
    
    // Asegurar que el estado esté correcto
    if (strlen(nuevaCita.estado) == 0) {
        strcpy(nuevaCita.estado, "Agendada");
    }
    
    // Abrir archivo en modo append
    ofstream archivo("citas.bin", ios::binary | ios::app);
    if (!archivo.is_open()) return false;
    
    archivo.write((char*)&nuevaCita, sizeof(Cita));
    archivo.close();
    
    // Actualizar header
    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    actualizarHeaderCitas(header);
    
    // Actualizar contador del hospital
    actualizarContadorHospital("cita");
    
    return true;
}

// Leer todas las citas activas (no eliminadas)
Cita* leerTodasCitasActivas(int* cantidadActivos = nullptr) {
    ifstream archivo("citas.bin", ios::binary);
    if (!archivo.is_open()) {
        if (cantidadActivos) *cantidadActivos = 0;
        return nullptr;
    }
    
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    if (header.cantidadRegistros == 0) {
        archivo.close();
        if (cantidadActivos) *cantidadActivos = 0;
        return nullptr;
    }
    
    // Contar cuántas citas no están eliminadas
    Cita temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    int activas = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Cita));
        if (!temp.eliminada) {
            activas++;
        }
    }
    
    if (activas == 0) {
        archivo.close();
        if (cantidadActivos) *cantidadActivos = 0;
        return nullptr;
    }
    
    // Crear array para citas activas
    Cita* citasActivas = new Cita[activas];
    
    // Volver al inicio y guardar las activas
    archivo.seekg(sizeof(ArchivoHeader));
    int indice = 0;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Cita));
        if (!temp.eliminada) {
            citasActivas[indice] = temp;
            indice++;
        }
    }
    
    archivo.close();
    
    if (cantidadActivos) {
        *cantidadActivos = activas;
    }
    
    return citasActivas;
}

// Buscar citas por estado
Cita* buscarCitasPorEstado(const char* estado, int* cantidadResultados) {
	
    *cantidadResultados = 0;
    
    if (estado == nullptr || strlen(estado) == 0) {
        return nullptr;
    }
    
    // Leer todas las citas activas
    int totalCitas = 0;
    Cita* todasCitas = leerTodasCitasActivas(&totalCitas);
    
    if (todasCitas == nullptr || totalCitas == 0) {
        return nullptr;
    }
    
    // Contar cuántas citas tienen el estado buscado
    int contador = 0;
    for (int i = 0; i < totalCitas; i++) {
        if (strcmp(todasCitas[i].estado, estado) == 0) {
            contador++;
        }
    }
    
    if (contador == 0) {
        delete[] todasCitas;
        *cantidadResultados = 0;
        return nullptr;
    }
    
    Cita* resultados = new Cita[contador];
    int index = 0;
    
    for (int i = 0; i < totalCitas; i++) {
        if (strcmp(todasCitas[i].estado, estado) == 0) {
            resultados[index++] = todasCitas[i];
        }
    }
    
    delete[] todasCitas;
    *cantidadResultados = contador;
    return resultados;
}

// Buscando citas por ID de paciente
Cita* buscarCitasPorPaciente(int idPaciente, int* cantidadResultados) {
    *cantidadResultados = 0;
    
    // Leyendo todas las citas activas
    int totalCitas = 0;
    Cita* todasCitas = leerTodasCitasActivas(&totalCitas);
    
    if (todasCitas == nullptr || totalCitas == 0) {
        return nullptr;
    }
    
    // Contando cuántas citas tiene el paciente
    int contador = 0;
    for (int i = 0; i < totalCitas; i++) {
        if (todasCitas[i].idPaciente == idPaciente) {
            contador++;
        }
    }
    
    if (contador == 0) {
        delete[] todasCitas;
        *cantidadResultados = 0;
        return nullptr;
    }
    
    Cita* resultados = new Cita[contador];
    int index = 0;
    
    for (int i = 0; i < totalCitas; i++) {
        if (todasCitas[i].idPaciente == idPaciente) {
            resultados[index++] = todasCitas[i];
        }
    }
    
    delete[] todasCitas;
    *cantidadResultados = contador;
    return resultados;
}

// Buscar citas por ID de doctor
Cita* buscarCitasPorDoctor(int idDoctor, int* cantidadResultados) {
	
    *cantidadResultados = 0;
    
    // Leer todas las citas activas
    int totalCitas = 0;
    Cita* todasCitas = leerTodasCitasActivas(&totalCitas);
    
    if (todasCitas == nullptr || totalCitas == 0) {
        return nullptr;
    }
    
    // Contar cuántas citas tiene el doctor
    int contador = 0;
    for (int i = 0; i < totalCitas; i++) {
        if (todasCitas[i].idDoctor == idDoctor) {
            contador++;
        }
    }
    
    if (contador == 0) {
        delete[] todasCitas;
        *cantidadResultados = 0;
        return nullptr;
    }
    
    // Crear array con las citas del doctor
    Cita* resultados = new Cita[contador];
    int index = 0;
    
    for (int i = 0; i < totalCitas; i++) {
        if (todasCitas[i].idDoctor == idDoctor) {
            resultados[index++] = todasCitas[i];
        }
    }
    
    delete[] todasCitas;
    *cantidadResultados = contador;
    return resultados;
}

// Buscar citas por fecha
Cita* buscarCitasPorFecha(const char* fecha, int* cantidadResultados) {
	
    *cantidadResultados = 0;
    
    if (fecha == nullptr || strlen(fecha) == 0) {
        return nullptr;
    }
    
    // Leer todas las citas activas
    int totalCitas = 0;
    Cita* todasCitas = leerTodasCitasActivas(&totalCitas);
    
    if (todasCitas == nullptr || totalCitas == 0) {
        return nullptr;
    }
    
    // Contar cuántas citas hay en esa fecha
    int contador = 0;
    for (int i = 0; i < totalCitas; i++) {
        if (strcmp(todasCitas[i].fecha, fecha) == 0) {
            contador++;
        }
    }
    
    if (contador == 0) {
        delete[] todasCitas;
        *cantidadResultados = 0;
        return nullptr;
    }
    
    // Crear array con las citas de esa fecha
    Cita* resultados = new Cita[contador];
    int index = 0;
    
    for (int i = 0; i < totalCitas; i++) {
        if (strcmp(todasCitas[i].fecha, fecha) == 0) {
            resultados[index++] = todasCitas[i];
        }
    }
    
    delete[] todasCitas;
    *cantidadResultados = contador;
    return resultados;
}

// Verificar disponibilidad del doctor (VERSIÓN ARCHIVOS)
bool verificarDisponibilidadArchivos(int idDoctor, const char* fecha, const char* hora) {
	
    // Buscar citas agendadas para el doctor
    int cantidadCitas = 0;
    Cita* citasDoctor = buscarCitasPorEstado("Agendada", &cantidadCitas);
	
    if (citasDoctor == nullptr || cantidadCitas == 0) {
        return true; // No hay citas agendadas, está disponible
    }
    
    // Verificar si el doctor tiene una cita en la misma fecha y hora
    for (int i = 0; i < cantidadCitas; i++) {
        Cita cita = citasDoctor[i];
        
        if (cita.idDoctor == idDoctor && 
            strcmp(cita.fecha, fecha) == 0 && 
            strcmp(cita.hora, hora) == 0) {
            delete[] citasDoctor;
            return false; // No está disponible
        }
    }
    
    delete[] citasDoctor;
    return true; // Está disponible
}

// Crear cita (EN MEMORIA)
Cita crearCita(int pacienteID, int doctorID, const char* fecha, 
               const char* hora, const char* motivo) {
    
    Cita nuevaCita;
    
    // Inicializar con valores por defecto
    memset(&nuevaCita, 0, sizeof(Cita));
    
    // Configurar datos básicos
    nuevaCita.idPaciente = pacienteID;
    nuevaCita.idDoctor = doctorID;
    strncpy(nuevaCita.fecha, fecha, 10);
    strncpy(nuevaCita.hora, hora, 5);
    strncpy(nuevaCita.motivo, motivo, 149);
    
    // Configurar estado
    strcpy(nuevaCita.estado, "Agendada");
    strcpy(nuevaCita.observaciones, "");
    nuevaCita.atendida = false;
    nuevaCita.eliminada = false;
    nuevaCita.id = -1;  // Sin consulta aún
    
    // Timestamps (se completarán al guardar)
    nuevaCita.fechaCreacion = 0;
    nuevaCita.fechaModificacion = 0;
    
    return nuevaCita;
}

// Registrar cita completa (FUNCIÓN PRINCIPAL)
bool registrarCitaCompleta(int pacienteID, int doctorID, const char* fecha,
                          const char* hora, const char* motivo) {
    
    // Verificando que el paciente existe y está activo
    Paciente paciente = buscarPacientePorID(pacienteID);
    if (paciente.id == -1 || !paciente.activo || paciente.eliminado) {
        return false;
    }
    
    // Verificando que el doctor existe y está activo
    Doctor doctor = buscarDoctorPorID(doctorID);
    if (doctor.id == -1 || !doctor.activo || doctor.eliminado) {
        return false;
    }
    
    // Validando fecha y hora
    if (!validarFecha(fecha) || !validarHora(hora)) {
        return false;
    }
    
    // Verificando disponibilidad del doctor
    if (!verificarDisponibilidadArchivos(doctorID, fecha, hora)) {
        return false;
    }
    
    // Verificando si hay espacio en el array del paciente
    if (paciente.cantidadCitas >= 20) {
        return false; // No hay espacio
    }
    
    // Verificando si hay espacio en el array del doctor
    if (doctor.cantidadCitas >= 30) {
        return false; // No hay espacio
    }
    
    // Creando la cita en memoria
    Cita nuevaCita = crearCita(pacienteID, doctorID, fecha, hora, motivo);
    
    // Guardando la cita en archivo
    bool resultadoCita = agregarCita(nuevaCita);
    if (!resultadoCita) {
        return false;
    }
    
    // Actualizando paciente (agregar ID de cita)
    // Obtener el ID de la cita recién creada
    ArchivoHeader headerCitas = leerHeaderCitas();
    int idCita = headerCitas.proximoID - 1;
    
    // Agregando ID de cita al array del paciente
    paciente.citasIDs[paciente.cantidadCitas] = idCita;
    paciente.cantidadCitas++;
    paciente.fechaModificacion = time(0);
    
    // Actualizando doctor (agregar ID de cita)
    doctor.citasIDs[doctor.cantidadCitas] = idCita;
    doctor.cantidadCitas++;
    doctor.fechaModificacion = time(0);
    
    if (!actualizarDoctor(doctor)) {
        // Intentar compensar
        return false;
    }
    
    return true;
}

// Cancelar cita (VERSIÓN ARCHIVOS)
bool cancelarCita(int idCita) {
    // Buscando la cita por ID
    Cita cita = buscarCitaPorID(idCita);
    if (cita.id == -1) {
        return false; // Cita no encontrada
    }
    
    // Verificando que esté en estado "Agendada"
    if (strcmp(cita.estado, "Agendada") != 0) {
        return false; // Ya fue atendida o cancelada
    }
    
    // Cambiando estado a "Cancelada"
    strcpy(cita.estado, "Cancelada");
    cita.atendida = false;
    
    // Actualizando la cita en archivo
    if (!actualizarCita(cita)) {
        return false;
    }
    
    // Removiendo ID de cita del array del paciente
    Paciente paciente = buscarPacientePorID(cita.idPaciente);
    if (paciente.id != -1) {
        // Buscar y remover el ID de la cita
        for (int i = 0; i < paciente.cantidadCitas; i++) {
            if (paciente.citasIDs[i] == idCita) {
                // Compactar array
                for (int j = i; j < paciente.cantidadCitas - 1; j++) {
                    paciente.citasIDs[j] = paciente.citasIDs[j + 1];
                }
                paciente.citasIDs[paciente.cantidadCitas - 1] = -1;
                paciente.cantidadCitas--;
                paciente.fechaModificacion = time(0);
                actualizarPaciente(paciente);
                break;
            }
        }
    }
    
    // Removiendo ID de cita del array del doctor
    Doctor doctor = buscarDoctorPorID(cita.idDoctor);
    if (doctor.id != -1) {
        // Buscar y remover el ID de la cita
        for (int i = 0; i < doctor.cantidadCitas; i++) {
            if (doctor.citasIDs[i] == idCita) {
                // Compactar array
                for (int j = i; j < doctor.cantidadCitas - 1; j++) {
                    doctor.citasIDs[j] = doctor.citasIDs[j + 1];
                }
                doctor.citasIDs[doctor.cantidadCitas - 1] = -1;
                doctor.cantidadCitas--;
                doctor.fechaModificacion = time(0);
                actualizarDoctor(doctor);
                break;
            }
        }
    }
    
    return true;
}

// Marcar cita como atendida (sin crear historial aún)
bool marcarCitaComoAtendida(int idCita, const char* observaciones) {
    // Buscando la cita por ID
    Cita cita = buscarCitaPorID(idCita);
    if (cita.id == -1) {
        return false; // Cita no encontrada
    }
    
    // Verificando que esté en estado "Agendada"
    if (strcmp(cita.estado, "Agendada") != 0) {
        return false; // Ya fue atendida o cancelada
    }
    
    // Cambiando estado a "Atendida"
    strcpy(cita.estado, "Atendida");
    cita.atendida = true;
    
    // Agregando observaciones si se proporcionaron
    if (observaciones != nullptr && strlen(observaciones) > 0) {
        strncpy(cita.observaciones, observaciones, 199);
    }
    
    // Actualizando la cita en archivo
    return actualizarCita(cita);
}

// Eliminar cita (borrado lógico)
bool eliminarCita(int idCita) {
    // Buscando la cita por ID
    Cita cita = buscarCitaPorID(idCita);
    if (cita.id == -1) {
        return false; // Cita no encontrada
    }
    
    // Verificando que no esté ya eliminada
    if (cita.eliminada) {
        return false; // Ya estaba eliminada
    }
    
    // Marcando como eliminada
    cita.eliminada = true;
    strcpy(cita.estado, "Eliminada");
    
    // Actualizando header (disminuyendo registros activos)
    ArchivoHeader header = leerHeaderCitas();
    header.registrosActivos--;
    
    // Guardando cambios
    if (!actualizarCita(cita)) {
        return false;
    }
    
    return actualizarHeaderCitas(header);
}

//---------------------------------------------
// AGENDAR CITA (INTERFAZ DE USUARIO) - VERSIÓN ARCHIVOS
//---------------------------------------------
void agendarCitaVisual() {
    // Verificar si hay pacientes y doctores en el sistema
    ArchivoHeader headerPacientes = leerHeaderPacientes();
    ArchivoHeader headerDoctores = leerHeaderDoctores();
    
    if (headerPacientes.registrosActivos == 0 || headerDoctores.registrosActivos == 0) {
        system("cls");
        system("color 1F");
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
    textoColor(10, 6, "Cédula del Paciente: ", 15, 1);
    textoColor(10, 7, "Cédula del Doctor: ", 15, 1);
    textoColor(10, 8, "Fecha (DD-MM-YYYY): ", 15, 1);
    textoColor(10, 9, "Hora (HH:MM): ", 15, 1);
    textoColor(10, 10, "Motivo: ", 15, 1);
    
    textoColor(10, 12, "Ejemplo: 15-01-2025, 09:30", 8, 1);
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
    Paciente paciente;
    Doctor doctor;
    
    // Cédula del Paciente
    do {
        temp = leerCampo(32, 6, 19);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            paciente = buscarPacientePorCedula(temp.c_str());
            if (paciente.id != -1 && paciente.activo && !paciente.eliminado) {
                strcpy(cedulaPaciente, temp.c_str());
                break;
            } else {
                mostrarError(10, 15, "Error: No se encontró paciente activo con esa cédula!");
            }
        } else {
            mostrarError(10, 15, "Error: La cédula del paciente no puede estar vacía!");
        }
    } while (true);
    
    // Cédula del Doctor
    do {
        temp = leerCampo(32, 7, 19);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            doctor = buscarDoctorPorCedula(temp.c_str());
            if (doctor.id != -1 && doctor.activo && !doctor.eliminado) {
                strcpy(cedulaDoctor, temp.c_str());
                break;
            } else {
                mostrarError(10, 15, "Error: No se encontró doctor activo con esa cédula!");
            }
        } else {
            mostrarError(10, 15, "Error: La cédula del doctor no puede estar vacía!");
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
                mostrarError(10, 15, "Error: Formato de fecha inválido! Use DD-MM-YYYY");
            }
        } else {
            mostrarError(10, 15, "Error: La fecha no puede estar vacía!");
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
                mostrarError(10, 15, "Error: Formato de hora inválido! Use HH:MM");
            }
        } else {
            mostrarError(10, 15, "Error: La hora no puede estar vacía!");
        }
    } while (true);
    
    // Motivo
    do {
        temp = leerCampo(32, 10, 149);
        if (temp == "ESC") return;
        
        if (!temp.empty()) {
            strcpy(motivo, temp.c_str());
            break;
        } else {
            mostrarError(10, 15, "Error: El motivo no puede estar vacío!");
        }
    } while (true);
    
    // Verificar disponibilidad del doctor usando archivos
    if (!verificarDisponibilidadArchivos(doctor.id, fecha, hora)) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "AGENDAR CITA", 14, 1);
        textoColor(10, 5, "Error: El doctor no está disponible en esa fecha y hora!", 12, 1);
        textoColor(10, 6, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
        textoColor(10, 7, "Fecha: " + string(fecha) + " Hora: " + string(hora), 15, 1);
        textoColor(10, 8, "Por favor, seleccione otra fecha u horario.", 14, 1);
        textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // AGENDAR LA CITA usando archivos
    bool resultado = registrarCitaCompleta(paciente.id, doctor.id, fecha, hora, motivo);
    
    // MOSTRAR RESULTADO
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "AGENDAR CITA", 14, 1);
    
    if (resultado) {
        ArchivoHeader headerCitas = leerHeaderCitas();
        textoColor(10, 5, "¡Cita agendada exitosamente!", 10, 1);
        textoColor(10, 6, "ID de Cita: " + to_string(headerCitas.proximoID - 1), 15, 1);
        textoColor(10, 7, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
        textoColor(10, 8, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
        textoColor(10, 9, "Fecha: " + string(fecha) + " Hora: " + string(hora), 15, 1);
        textoColor(10, 10, "Motivo: " + string(motivo), 15, 1);
    } else {
        textoColor(10, 5, "Error: No se pudo agendar la cita!", 12, 1);
        textoColor(10, 6, "Posibles causas:", 14, 1);
        textoColor(10, 7, "- El paciente ya tiene demasiadas citas (límite: 20)", 14, 1);
        textoColor(10, 8, "- El doctor ya tiene demasiadas citas (límite: 30)", 14, 1);
        textoColor(10, 9, "- Error en el sistema de archivos", 14, 1);
        textoColor(10, 10, "Por favor, verifique los datos e intente nuevamente.", 14, 1);
    }
    
    textoColor(10, 11, "Presione cualquier tecla para continuar...", 14, 1);
    _getch();
}

//---------------------------------------------
// MOSTRAR CITAS EN TABLA (VERSIÓN ARCHIVOS)
//---------------------------------------------
void mostrarCitasEnTabla(Cita** citas, int cantidad, const string& titulo) {
    
    int startX = -6; 
    int startY = 0;
    
    system("cls");
    system("color 1F");
    
    int y = startY;
    
    // CABECERA DE LA TABLA 
    textoColor(startX + 10, y++, ES_SU_IZ + replicar(LINEA_HO[0], 72) + ES_SU_DE, 14, 1);
    textoColor(startX + 10, y, LINEA_VE + "                    " + titulo, 14, 1);
    textoColor(startX + 83, y++, LINEA_VE, 14, 1);
    textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 19) + CRUZ_SUP + replicar(LINEA_HO[0], 19) + CRUZ_SUP + replicar(LINEA_HO[0], 12) + CRUZ_SUP + replicar(LINEA_HO[0], 7) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_DER, 14, 1);
    
    // ENCABEZADO DE COLUMNAS 
    textoColor(startX + 12, y, "ID", 14, 1);
    textoColor(startX + 18, y, "Paciente", 14, 1);
    textoColor(startX + 38, y, "Doctor", 14, 1);
    textoColor(startX + 57, y, "Fecha", 14, 1);
    textoColor(startX + 70, y, "Hora", 14, 1);
    textoColor(startX + 77, y, "Estado", 14, 1);

    // Líneas verticales del encabezado 
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
        
        // Obtener nombres del paciente y doctor desde archivos
        Paciente paciente = buscarPacientePorID(cita->idPaciente);
        Doctor doctor = buscarDoctorPorID(cita->idDoctor);
        
        string nombrePaciente = "No encontrado";
        string nombreDoctor = "No encontrado";
        
        if (paciente.id != -1 && paciente.activo && !paciente.eliminado) {
            nombrePaciente = string(paciente.nombre) + " " + string(paciente.apellido);
        }
        
        if (doctor.id != -1 && doctor.activo && !doctor.eliminado) {
            nombreDoctor = "Dr. " + string(doctor.nombre) + " " + string(doctor.apellido);
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
        } else if (strcmp(cita->estado, "Eliminada") == 0) {
            estadoAbreviado = "ELIM";
            colorEstado = 8; // Gris
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
        
        // Líneas verticales 
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
            textoColor(startX + 15, y++, "Presione cualquier tecla para ver más...", 14, 1);
            _getch();
            
            // Nueva página
            system("cls");
            system("color 1F");
            y = startY;
            textoColor(startX + 10, y++, ES_SU_IZ + replicar(LINEA_HO[0], 72) + ES_SU_DE, 14, 1);
            textoColor(startX + 10, y, LINEA_VE + "                    " + titulo + " (Cont.)", 14, 1);
            textoColor(startX + 83, y++, LINEA_VE, 14, 1);
            
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_SUP + replicar(LINEA_HO[0], 19) + CRUZ_SUP + replicar(LINEA_HO[0], 19) + CRUZ_SUP + replicar(LINEA_HO[0], 12) + CRUZ_SUP + replicar(LINEA_HO[0], 7) + CRUZ_SUP + replicar(LINEA_HO[0], 6) + CRUZ_DER, 14, 1);
            
            // Re-dibujar encabezado
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
            textoColor(startX + 83, y, LINEA_VE, 14, 1);
            y++;
            
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 4) + CRUZ_CEN + replicar(LINEA_HO[0], 19) + CRUZ_CEN + replicar(LINEA_HO[0], 19) + CRUZ_CEN + replicar(LINEA_HO[0], 12) + CRUZ_CEN + replicar(LINEA_HO[0], 7) + CRUZ_CEN + replicar(LINEA_HO[0], 6) + CRUZ_DER, 14, 1);
        }
    }
    
    // PIE DE TABLA 
    textoColor(startX + 10, y++, ES_IN_IZ + replicar(LINEA_HO[0], 4) + CRUZ_INF + replicar(LINEA_HO[0], 19) + CRUZ_INF + replicar(LINEA_HO[0], 19) + CRUZ_INF + replicar(LINEA_HO[0], 12) + CRUZ_INF + replicar(LINEA_HO[0], 7) + CRUZ_INF + replicar(LINEA_HO[0], 6) + ES_IN_DE, 14, 1);
    textoColor(startX + 15, y++, "Total encontradas: " + to_string(cantidad), 14, 1);
    textoColor(startX + 15, y++, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
}


//---------------------------------------------
// LISTAR CITAS PENDIENTES
//---------------------------------------------
void listarCitasPendientes() {
    // Verificar si hay citas en el sistema
    ArchivoHeader headerCitas = leerHeaderCitas();
    if (headerCitas.registrosActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS PENDIENTES", 14, 1);
        textoColor(10, 5, "No hay citas registradas en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    // Obtener citas pendientes desde archivo
    int cantidadResultados = 0;
    Cita* citasPendientesArray = buscarCitasPorEstado("Agendada", &cantidadResultados);
    
    if (cantidadResultados == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS PENDIENTES", 14, 1);
        textoColor(10, 5, "¡Excelente! No hay citas pendientes en el sistema.", 10, 1);
        textoColor(10, 6, "Todas las citas han sido atendidas o canceladas.", 10, 1);
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    Cita** citasPendientes = new Cita*[cantidadResultados];
    for (int i = 0; i < cantidadResultados; i++) {
        citasPendientes[i] = &citasPendientesArray[i];
    }
    
    // Mostrar resultados en tabla
    string titulo = "CITAS PENDIENTES (" + to_string(cantidadResultados) + ")";
    mostrarCitasEnTabla(citasPendientes, cantidadResultados, titulo);
    
    // Liberar memoria
    delete[] citasPendientesArray;
    delete[] citasPendientes;
}

//---------------------------------------------
// FUNCIONES PARA HISTORIAL MÉDICO (ARCHIVOS)
//---------------------------------------------

// Agregar historial médico al archivo
bool agregarHistorialMedico(HistorialMedico& historial) {
    ArchivoHeader header = leerHeaderConsultas();
    
    // Configurar datos del historial
    historial.id = header.proximoID;  // ID del historial médico
    historial.eliminado = false;
    
    // Timestamps
    time_t now = time(0);
    historial.fechaCreacion = now;
    historial.fechaModificacion = now;
    
    // Verificar que tenga un ID de consulta válido
    if (historial.idConsulta <= 0) {
        historial.idConsulta = -1;  // Si no tiene consulta asociada
    }
    
    // Abrir archivo en modo append
    ofstream archivo("historial.bin", ios::binary | ios::app);
    if (!archivo.is_open()) return false;
    
    archivo.write((char*)&historial, sizeof(HistorialMedico));
    archivo.close();
    
    // Actualizar header usando la función apropiada
    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    
    // Usar función para actualizar header
    fstream archivoHeader("historial.bin", ios::binary | ios::in | ios::out);
    if (!archivoHeader.is_open()) return false;
    
    archivoHeader.seekp(0);
    archivoHeader.write((char*)&header, sizeof(ArchivoHeader));
    archivoHeader.close();
    
    // Actualizar contador del hospital
    actualizarContadorHospital("consulta");
    
    return true;
}
// Buscar historial por ID
HistorialMedico buscarHistorialPorID(int idConsulta) {
	
    ifstream archivo("historial.bin", ios::binary);
    if (!archivo.is_open()) {
        HistorialMedico vacio;
        vacio.idConsulta = -1;
        return vacio;
    }
    
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    HistorialMedico temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(HistorialMedico));
        if (temp.idConsulta == idConsulta && !temp.eliminado) {
            archivo.close();
            return temp;
        }
    }
    
    archivo.close();
    HistorialMedico vacio;
    vacio.idConsulta = -1;
    return vacio;
}

// Buscar historial por ID de paciente
HistorialMedico* buscarHistorialPorPaciente(int idPaciente, int* cantidad) {
    *cantidad = 0;
    
    ifstream archivo("historial.bin", ios::binary);
    if (!archivo.is_open()) return nullptr;
    
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    // Contar cuántas consultas tiene el paciente
    HistorialMedico temp;
    archivo.seekg(sizeof(ArchivoHeader));
    
    int contador = 0;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(HistorialMedico));
        if (temp.idPaciente == idPaciente && !temp.eliminado) {
            contador++;
        }
    }
    
    if (contador == 0) {
        archivo.close();
        return nullptr;
    }
    
    HistorialMedico* resultados = new HistorialMedico[contador];
    archivo.seekg(sizeof(ArchivoHeader));
    int index = 0;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(HistorialMedico));
        if (temp.idPaciente == idPaciente && !temp.eliminado) {
            resultados[index++] = temp;
        }
    }
    
    archivo.close();
    *cantidad = contador;
    return resultados;
}


//---------------------------------------------
// ATENDER CITA (CREA HISTORIAL MEDICO) 
//---------------------------------------------
bool atenderCita(int idCita, const char* diagnostico,
                 const char* tratamiento, const char* medicamentos) {
    
    if (diagnostico == nullptr || tratamiento == nullptr || medicamentos == nullptr) {
        return false;
    }
    
    // Buscando la cita por ID en archivo
    Cita cita = buscarCitaPorID(idCita);
    
    if (cita.id == -1) {
        return false;
    }
    
    // Verificando que esté en estado "Agendada"
    if (strcmp(cita.estado, "Agendada") != 0) {
        return false; // Ya fue atendida o cancelada
    }
    
    // Obteniendo paciente y doctor desde archivos
    Paciente paciente = buscarPacientePorID(cita.idPaciente);
    Doctor doctor = buscarDoctorPorID(cita.idDoctor);
    
    if (paciente.id == -1 || doctor.id == -1) {
        return false;
    }
    
    // Creando estructura HistorialMedico
    HistorialMedico nuevaConsulta;
    
    // Obtener próximo ID de consulta
    ArchivoHeader headerConsultas = leerHeaderConsultas();
    nuevaConsulta.idConsulta = headerConsultas.proximoID;
    
    strcpy(nuevaConsulta.fecha, cita.fecha);
    strcpy(nuevaConsulta.hora, cita.hora);
    strcpy(nuevaConsulta.diagnostico, diagnostico);
    strcpy(nuevaConsulta.tratamiento, tratamiento);
    strcpy(nuevaConsulta.medicamentos, medicamentos);
    nuevaConsulta.idDoctor = cita.idDoctor;
    nuevaConsulta.costo = doctor.costoConsulta;
    
    // Metadata del historial
    nuevaConsulta.eliminado = false;
    time_t now = time(0);
    nuevaConsulta.fechaCreacion = now;
    nuevaConsulta.fechaModificacion = now;
    nuevaConsulta.idConsulta = cita.id;
    nuevaConsulta.idPaciente = cita.idPaciente;
    
    // Guardando historial en archivo
    if (!agregarHistorialMedico(nuevaConsulta)) {
        return false;
    }
    
    // Actualizar cita - marcar como atendida y vincular consulta
    strcpy(cita.estado, "Atendida");
    cita.atendida = true;
    cita.id = nuevaConsulta.idConsulta;
    cita.fechaModificacion = now;
    
    // Agregar observaciones con resumen del diagnóstico
    char obsTemp[200];
    snprintf(obsTemp, sizeof(obsTemp), "Atendida - Diagnóstico: %s", diagnostico);
    strncpy(cita.observaciones, obsTemp, 199);
    
    if (!actualizarCita(cita)) {
        return false;
    }
    
    // Actualizar paciente - incrementar contador de consultas
    paciente.cantidadConsultas++;
    paciente.fechaModificacion = now;
    
    // Si es la primera consulta, guardar el ID
    if (paciente.primerConsultaID == -1) {
        paciente.primerConsultaID = nuevaConsulta.idConsulta;
    }
    
    if (!actualizarPaciente(paciente)) {
        return false;
    }
    
    return true;
}

//---------------------------------------------
// ATENDER CITA (INTERFAZ DE USUARIO) - VERSIÓN ARCHIVOS
//---------------------------------------------
void atenderCitaVisual() {
    // Verificar si hay citas en el sistema
    ArchivoHeader headerCitas = leerHeaderCitas();
    if (headerCitas.registrosActivos == 0) {
        system("cls");
        system("color 1F");
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
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        textoColor(10, 5, "Error: El ID no puede estar vacio!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    int idCita = atoi(temp.c_str());
    
    // Buscar la cita en archivo
    Cita cita = buscarCitaPorID(idCita);
    
    if (cita.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        textoColor(10, 5, "Error: No se encontro la cita con ID: " + to_string(idCita), 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Verificar que la cita esté en estado agendada
    if (strcmp(cita.estado, "Agendada") != 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        textoColor(10, 5, "Error: La cita no esta en estado 'Agendada'!", 12, 1);
        textoColor(10, 6, "ID Cita: " + to_string(cita.id), 15, 1);
        textoColor(10, 7, "Estado actual: " + string(cita.estado), 15, 1);
        textoColor(10, 8, "Solo se pueden atender citas en estado 'Agendada'", 14, 1);
        textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Obtener información del paciente y doctor desde archivos
    Paciente paciente = buscarPacientePorID(cita.idPaciente);
    Doctor doctor = buscarDoctorPorID(cita.idDoctor);
    
    if (paciente.id == -1 || doctor.id == -1) {
        system("cls");
        system("color 1F");
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
    textoColor(10, 6, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
    textoColor(10, 7, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
    textoColor(10, 8, "Fecha: " + string(cita.fecha) + " Hora: " + string(cita.hora), 15, 1);
    textoColor(10, 9, "Motivo: " + string(cita.motivo), 15, 1);
    textoColor(10, 10, "Costo de consulta: $" + to_string(doctor.costoConsulta), 15, 1);
    
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
    
    // Diagnóstico
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
    system("color 1F");
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "CONFIRMAR ATENCIÓN", 14, 1);
    textoColor(10, 5, "¿Confirmar atención de la cita?", 15, 1);
    textoColor(10, 6, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
    textoColor(10, 7, "Diagnostico: " + string(diagnostico), 15, 1);
    textoColor(10, 8, "Tratamiento: " + string(tratamiento), 15, 1);
    textoColor(10, 9, "Costo: $" + to_string(doctor.costoConsulta), 15, 1);
    textoColor(10, 10, "Presione S para confirmar, cualquier otra tecla para cancelar", 14, 1);
    
    char confirmacion = _getch();
    
    if (confirmacion == 'S' || confirmacion == 's') {
        // Atender la cita (esto crea el historial medico automaticamente)
        bool resultado = atenderCita(idCita, diagnostico, tratamiento, medicamentos);
        
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "ATENDER CITA", 14, 1);
        
        if (resultado) {
            // Obtener el ID de consulta recién creado
            ArchivoHeader headerConsultas = leerHeaderConsultas();
            int idConsulta = headerConsultas.proximoID - 1;
            
            textoColor(10, 5, "¡Cita atendida exitosamente!", 10, 1);
            textoColor(10, 6, "Se ha generado el historial médico del paciente.", 10, 1);
            textoColor(10, 7, "ID de Consulta: " + to_string(idConsulta), 15, 1);
            textoColor(10, 8, "Estado actualizado: ATEN", 15, 1);
        } else {
            textoColor(10, 5, "Error: No se pudo atender la cita!", 12, 1);
        }
        
        textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
    }
}


//---------------------------------------------
// CANCELAR CITA (INTERFAZ DE USUARIO) - VERSIÓN ARCHIVOS
//---------------------------------------------
void cancelarCitaVisual() {
    // Verificar si hay citas en el sistema
    ArchivoHeader headerCitas = leerHeaderCitas();
    if (headerCitas.registrosActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CANCELAR CITA", 14, 1);
        textoColor(10, 5, "Error: No hay citas registradas en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 10);
    
    textoColor(25, 3, "CANCELAR CITA", 14, 1);
    textoColor(10, 5, "Ingrese el ID de la cita a cancelar: ", 15, 1);
    textoColor(10, 7, "Nota: Solo se pueden cancelar citas en estado 'AGEN'", 14, 1);
    textoColor(10, 9, "Presione ESC para cancelar", 14, 1);
    
    // Leer ID de la cita
    string temp = leerCampoNumerico(46, 5, 5);
    
    if (temp == "ESC") {
        return; 
    }
    
    if (temp.empty()) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CANCELAR CITA", 14, 1);
        textoColor(10, 5, "Error: El ID no puede estar vacio!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    int idCita = atoi(temp.c_str());
    
    // Buscar la cita en archivo
    Cita cita = buscarCitaPorID(idCita);
    
    if (cita.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CANCELAR CITA", 14, 1);
        textoColor(10, 5, "Error: No se encontro la cita con ID: " + to_string(idCita), 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Verificar que la cita esté en estado agendada
    if (strcmp(cita.estado, "Agendada") != 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "CANCELAR CITA", 14, 1);
        textoColor(10, 5, "Error: La cita no esta en estado 'Agendada'!", 12, 1);
        textoColor(10, 6, "ID Cita: " + to_string(cita.id), 15, 1);
        textoColor(10, 7, "Estado actual: " + string(cita.estado), 15, 1);
        textoColor(10, 8, "Solo se pueden cancelar citas en estado 'Agendada'", 14, 1);
        textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Obtener información del paciente y doctor desde archivos
    Paciente paciente = buscarPacientePorID(cita.idPaciente);
    Doctor doctor = buscarDoctorPorID(cita.idDoctor);
    
    if (paciente.id == -1 || doctor.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CANCELAR CITA", 14, 1);
        textoColor(10, 5, "Error: No se pudo encontrar paciente o doctor!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // MOSTRAR INFORMACIÓN DE LA CITA Y CONFIRMAR
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 12);
    
    textoColor(25, 3, "CONFIRMAR CANCELACIÓN", 14, 1);
    textoColor(10, 5, "¿Está seguro que desea cancelar la siguiente cita?", 15, 1);
    textoColor(10, 6, "ID Cita: " + to_string(cita.id), 15, 1);
    textoColor(10, 7, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
    textoColor(10, 8, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
    textoColor(10, 9, "Fecha: " + string(cita.fecha) + " Hora: " + string(cita.hora), 15, 1);
    textoColor(10, 10, "Motivo: " + string(cita.motivo), 15, 1);
    textoColor(10, 11, "Presione S para confirmar, cualquier otra tecla para cancelar", 14, 1);
    
    char confirmacion = _getch();
    
    if (confirmacion == 'S' || confirmacion == 's') {
        // Cancelar la cita usando la función de archivos
        bool resultado = cancelarCita(idCita);
        
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 10);
        textoColor(25, 3, "CANCELAR CITA", 14, 1);
        
        if (resultado) {
            textoColor(10, 5, "¡Cita cancelada exitosamente!", 10, 1);
            textoColor(10, 6, "ID Cita: " + to_string(cita.id), 15, 1);
            textoColor(10, 7, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
            textoColor(10, 8, "Estado actualizado: CANC", 15, 1);
        } else {
            textoColor(10, 5, "Error: No se pudo cancelar la cita!", 12, 1);
        }
        
        textoColor(10, 9, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
    }
}
//---------------------------------------------
// OBTENER CITAS DE PACIENTE (INTERFAZ DE USUARIO) - VERSIÓN ARCHIVOS
//---------------------------------------------
void obtenerCitasDePacienteVisual() {
    // Verificar si hay pacientes en el sistema
    ArchivoHeader headerPacientes = leerHeaderPacientes();
    if (headerPacientes.registrosActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS DE PACIENTE", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    
    textoColor(25, 3, "CITAS DE PACIENTE", 14, 1);
    textoColor(10, 5, "Ingrese la cédula del paciente: ", 15, 1);
    textoColor(10, 7, "Presione ESC para cancelar", 14, 1);
    
    // Leer cédula del paciente
    string cedula = leerCampo(42, 5, 19);
    
    if (cedula == "ESC") {
        return; 
    }
    
    if (cedula.empty()) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS DE PACIENTE", 14, 1);
        textoColor(10, 5, "Error: La cédula no puede estar vacía!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Buscar paciente por cédula en archivo
    Paciente paciente = buscarPacientePorCedula(cedula.c_str());
    
    if (paciente.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS DE PACIENTE", 14, 1);
        textoColor(10, 5, "Error: No se encontró paciente con cédula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    if (!paciente.activo || paciente.eliminado) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS DE PACIENTE", 14, 1);
        textoColor(10, 5, "Error: El paciente está inactivo en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Obtener todas las citas del paciente desde archivo
    int cantidadCitas = 0;
    Cita* citasPaciente = buscarCitasPorPaciente(paciente.id, &cantidadCitas);
    
    if (cantidadCitas == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS DE PACIENTE", 14, 1);
        textoColor(10, 5, "El paciente no tiene citas registradas.", 12, 1);
        textoColor(10, 6, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Crear array de punteros para compatibilidad con mostrarCitasEnTabla
    Cita** citasPtr = new Cita*[cantidadCitas];
    for (int i = 0; i < cantidadCitas; i++) {
        citasPtr[i] = &citasPaciente[i];
    }
    
    // Mostrar citas en tabla
    string titulo = "CITAS DE: " + string(paciente.nombre) + " " + string(paciente.apellido);
    
    mostrarCitasEnTabla(citasPtr, cantidadCitas, titulo);
    
    // Liberar memoria
    delete[] citasPaciente;
    delete[] citasPtr;
}

///---------------------------------------------
// OBTENER CITAS DE DOCTOR (INTERFAZ DE USUARIO) - VERSIÓN ARCHIVOS
//---------------------------------------------
void obtenerCitasDeDoctorVisual() {
    // Verificar si hay doctores en el sistema
    ArchivoHeader headerDoctores = leerHeaderDoctores();
    if (headerDoctores.registrosActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS DE DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No hay doctores registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    
    textoColor(25, 3, "CITAS DE DOCTOR", 14, 1);
    textoColor(10, 5, "Ingrese la cédula del doctor: ", 15, 1);
    textoColor(10, 7, "Presione ESC para cancelar", 14, 1);
    
    // Leer cédula del doctor
    string cedula = leerCampo(42, 5, 19);
    
    if (cedula == "ESC") {
        return; 
    }
    
    if (cedula.empty()) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS DE DOCTOR", 14, 1);
        textoColor(10, 5, "Error: La cédula no puede estar vacía!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Buscar doctor por cédula en archivo
    Doctor doctor = buscarDoctorPorCedula(cedula.c_str());
    
    if (doctor.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS DE DOCTOR", 14, 1);
        textoColor(10, 5, "Error: No se encontró doctor con cédula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Obtener todas las citas del doctor desde archivo
    int cantidadCitas = 0;
    Cita* citasDoctor = buscarCitasPorDoctor(doctor.id, &cantidadCitas);
    
    if (cantidadCitas == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS DE DOCTOR", 14, 1);
        textoColor(10, 5, "El doctor no tiene citas registradas.", 12, 1);
        textoColor(10, 6, "Doctor: Dr. " + string(doctor.nombre) + " " + string(doctor.apellido), 15, 1);
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Crear array de punteros para compatibilidad
    Cita** citasPtr = new Cita*[cantidadCitas];
    for (int i = 0; i < cantidadCitas; i++) {
        citasPtr[i] = &citasDoctor[i];
    }
    
    // Mostrar citas en tabla
    string titulo = "CITAS DR: " + string(doctor.nombre) + " " + string(doctor.apellido);
    
    // Necesitamos una versión actualizada de mostrarCitasEnTabla que no use Hospital
    mostrarCitasEnTabla(citasPtr, cantidadCitas, titulo);
    
    // Liberar memoria
    delete[] citasDoctor;
    delete[] citasPtr;
}


//---------------------------------------------
// OBTENER CITAS POR FECHA (INTERFAZ DE USUARIO) - VERSIÓN ARCHIVOS
//---------------------------------------------
void obtenerCitasPorFechaVisual() {
    // Verificar si hay citas en el sistema
    ArchivoHeader headerCitas = leerHeaderCitas();
    if (headerCitas.registrosActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS POR FECHA", 14, 1);
        textoColor(10, 5, "Error: No hay citas registradas en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 10);
    
    textoColor(25, 3, "CITAS POR FECHA", 14, 1);
    textoColor(10, 5, "Ingrese la fecha (DD-MM-YYYY): ", 15, 1);
    textoColor(10, 7, "Ejemplo: 15-01-2025", 8, 1);
    textoColor(10, 9, "Presione ESC para cancelar", 14, 1);
    
    // Leer fecha
    string fecha = leerCampo(42, 5, 10);
    
    if (fecha == "ESC") {
        return; 
    }
    
    if (fecha.empty()) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS POR FECHA", 14, 1);
        textoColor(10, 5, "Error: La fecha no puede estar vacía!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Validar formato de fecha
    if (!validarFecha(fecha.c_str())) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS POR FECHA", 14, 1);
        textoColor(10, 5, "Error: Formato de fecha inválido! Use DD-MM-YYYY", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Obtener citas por fecha desde archivo
    int cantidadCitas = 0;
    Cita* citasFechaArray = buscarCitasPorFecha(fecha.c_str(), &cantidadCitas);
    
    if (cantidadCitas == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "CITAS POR FECHA", 14, 1);
        textoColor(10, 5, "No hay citas registradas para la fecha: " + fecha, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Crear array de punteros para compatibilidad
    Cita** citasFecha = new Cita*[cantidadCitas];
    for (int i = 0; i < cantidadCitas; i++) {
        citasFecha[i] = &citasFechaArray[i];
    }
    
    // Mostrar citas en tabla
    string tituloTabla = "CITAS DEL " + fecha;
    mostrarCitasEnTabla(citasFecha, cantidadCitas, tituloTabla);
    
    // Liberar memoria
    delete[] citasFechaArray;
    delete[] citasFecha;
}



//---------------------------------------------
// MOSTRAR HISTORIAL MÉDICO EN TABLA (VERSIÓN ARCHIVOS)
//---------------------------------------------
void mostrarHistorialMedicoEnTabla(const Paciente& paciente, const string& titulo) {
    
    int startX = -6; 
    int startY = 0;

    // Obtener el historial médico del paciente desde archivo
    int cantidadConsultas = 0;
    HistorialMedico* historialPaciente = buscarHistorialPorPaciente(paciente.id, &cantidadConsultas);
    
    if (cantidadConsultas == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "HISTORIAL MÉDICO", 14, 1);
        textoColor(10, 5, "El paciente no tiene consultas en su historial", 12, 1);
        textoColor(10, 6, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
        textoColor(10, 7, "Presione cualquier tecla para continuar...", 14, 1);
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
    textoColor(startX + 25, y, "Diagnóstico", 14, 1);
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
    
    // LISTA DE CONSULTAS (del más reciente al más antiguo)
    for (int i = cantidadConsultas - 1; i >= 0; i--) {
        HistorialMedico consulta = historialPaciente[i];
        
        // Obtener doctor desde archivo
        Doctor doctor = buscarDoctorPorID(consulta.idDoctor);
        string nombreDoctor = "N/E"; // No encontrado
        if (doctor.id != -1) {
            nombreDoctor = "Dr. " + string(doctor.apellido);
            // Acortar nombre si es muy largo
            if (nombreDoctor.length() > 10) {
                nombreDoctor = nombreDoctor.substr(0, 8) + ".";
            }
        }
        
        // Fecha y hora juntas 
        string fechaHora = string(consulta.fecha).substr(5, 5) + " " + string(consulta.hora); 
        
        // Acortar diagnóstico 
        string diagnosticoAbreviado = consulta.diagnostico;
        if (diagnosticoAbreviado.length() > 13) {
            diagnosticoAbreviado = diagnosticoAbreviado.substr(0, 11) + "..";
        }
        
        // Acortar tratamiento (incluye medicina si cabe)
        string tratamientoAbreviado = consulta.tratamiento;
        string medicamentos = consulta.medicamentos;
        
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
        
        // Diagnóstico (corto)
        textoColor(startX + 25, y, diagnosticoAbreviado, colorFila, 1);
        
        // Tratamiento (con medicina si cabe)
        textoColor(startX + 42, y, tratamientoAbreviado, colorFila, 1);
        
        // Doctor (nombre)
        textoColor(startX + 59, y, nombreDoctor, colorFila, 1);
        
        // Líneas verticales
        textoColor(startX + 23, y, LINEA_VE, 14, 1);
        textoColor(startX + 40, y, LINEA_VE, 14, 1);
        textoColor(startX + 57, y, LINEA_VE, 14, 1);
        textoColor(startX + 69, y, LINEA_VE, 14, 1);
        
        y++;
        
        // Paginación (cada 20 consultas)
        if (y >= (startY + 20) && i > 0) {
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 12) + CRUZ_INF + replicar(LINEA_HO[0], 15) + CRUZ_INF + replicar(LINEA_HO[0], 15) + CRUZ_INF + replicar(LINEA_HO[0], 14) + CRUZ_DER, 14, 1);
            textoColor(startX + 15, y++, "Presione cualquier tecla para ver más...", 14, 1);
            _getch();
            
            // Nueva página
            system("cls");
            system("color 1F");
            y = startY;
            textoColor(startX + 10, y++, ES_SU_IZ + replicar(LINEA_HO[0], 58) + ES_SU_DE, 14, 1);
            textoColor(startX + 10, y++, LINEA_VE + "              " + titulo + " (Cont.)          " + LINEA_VE, 14, 1);
            textoColor(startX + 10, y++, CRUZ_IZQ + replicar(LINEA_HO[0], 12) + CRUZ_SUP + replicar(LINEA_HO[0], 15) + CRUZ_SUP + replicar(LINEA_HO[0], 15) + CRUZ_SUP + replicar(LINEA_HO[0], 14) + CRUZ_DER, 14, 1);
            
            // Re-dibujar encabezado
            textoColor(startX + 12, y, "Fecha/Hora", 14, 1);
            textoColor(startX + 25, y, "Diagnóstico", 14, 1);
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
    textoColor(startX + 15, y++, "Total consultas: " + to_string(cantidadConsultas), 14, 1);
    textoColor(startX + 15, y++, "Paciente: " + string(paciente.nombre) + " " + string(paciente.apellido), 15, 1);
    textoColor(startX + 15, y++, "Presione cualquier tecla para continuar...", 14, 1);
    
    _getch();
    
    // Liberar memoria
    delete[] historialPaciente;
}

//---------------------------------------------
// MOSTRAR HISTORIAL MÉDICO (INTERFAZ DE USUARIO) - VERSIÓN ARCHIVOS
//---------------------------------------------
void mostrarHistorialMedicoVisual() {
    // Verificar si hay pacientes en el sistema
    ArchivoHeader headerPacientes = leerHeaderPacientes();
    if (headerPacientes.registrosActivos == 0) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "HISTORIAL MÉDICO", 14, 1);
        textoColor(10, 5, "Error: No hay pacientes registrados en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }

    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    
    textoColor(25, 3, "HISTORIAL MÉDICO", 14, 1);
    textoColor(10, 5, "Ingrese la cédula del paciente: ", 15, 1);
    textoColor(10, 7, "Presione ESC para cancelar", 14, 1);
    
    // Leer cédula del paciente
    string cedula = leerCampo(42, 5, 19);
    
    if (cedula == "ESC") {
        return; 
    }
    
    if (cedula.empty()) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "HISTORIAL MÉDICO", 14, 1);
        textoColor(10, 5, "Error: La cédula no puede estar vacía!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Buscar paciente por cédula en archivo
    Paciente paciente = buscarPacientePorCedula(cedula.c_str());
    
    if (paciente.id == -1) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "HISTORIAL MÉDICO", 14, 1);
        textoColor(10, 5, "Error: No se encontró paciente con cédula: " + cedula, 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    if (!paciente.activo || paciente.eliminado) {
        system("cls");
        system("color 1F");
        dibujarCuadro(2, 3, 75, 8);
        textoColor(25, 3, "HISTORIAL MÉDICO", 14, 1);
        textoColor(10, 5, "Error: El paciente está inactivo en el sistema!", 12, 1);
        textoColor(10, 6, "Presione cualquier tecla para continuar...", 14, 1);
        _getch();
        return;
    }
    
    // Mostrar historial médico
    string titulo = "HISTORIAL - " + string(paciente.nombre) + " " + string(paciente.apellido);
    mostrarHistorialMedicoEnTabla(paciente, titulo);
}



//  MOSTRAR INFORMACIÓN DEL HOSPITAL
void mostrarInfoHospital(Hospital& hosp) {
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 18);
    
    textoColor(25, 3, "INFORMACIÓN DEL HOSPITAL", 14, 1);
    
    // Información básica
    textoColor(10, 5, "Nombre: " + string(hosp.nombre), 15, 1);
    textoColor(10, 6, "Dirección: " + string(hosp.direccion), 15, 1);
    textoColor(10, 7, "Teléfono: " + string(hosp.telefono), 15, 1);
    
    // Contadores de IDs
    textoColor(10, 9, "CONTADORES DE ID:", 14, 1);
    textoColor(10, 10, "Siguiente ID Paciente: " + to_string(hosp.siguienteIDPaciente), 15, 1);
    textoColor(10, 11, "Siguiente ID Doctor: " + to_string(hosp.siguienteIDDoctor), 15, 1);
    textoColor(10, 12, "Siguiente ID Cita: " + to_string(hosp.siguienteIDCita), 15, 1);
    textoColor(10, 13, "Siguiente ID Consulta: " + to_string(hosp.siguienteIDConsulta), 15, 1);
    
    // Estadísticas
    textoColor(10, 15, "ESTADÍSTICAS:", 14, 1);
    textoColor(10, 16, "Total Pacientes: " + to_string(hosp.totalPacientesRegistrados), 15, 1);
    textoColor(10, 17, "Total Doctores: " + to_string(hosp.totalDoctoresRegistrados), 15, 1);
    textoColor(10, 18, "Total Citas: " + to_string(hosp.totalCitasAgendadas), 15, 1);
    textoColor(10, 19, "Total Consultas: " + to_string(hosp.totalConsultasRealizadas), 15, 1);
    
    textoColor(10, 21, "Presione cualquier tecla para continuar...", 14, 1);
    _getch();
}


// OBTENER NUEVO ID SEGÚN TIPO
int obtenerNuevoID(Hospital& hosp, const string& tipo) {
    int nuevoID = -1;
    
    if (tipo == "paciente") {
        nuevoID = hosp.siguienteIDPaciente;
    }
    else if (tipo == "doctor") {
        nuevoID = hosp.siguienteIDDoctor;
    }
    else if (tipo == "cita") {
        nuevoID = hosp.siguienteIDCita;
    }
    else if (tipo == "consulta") {
        nuevoID = hosp.siguienteIDConsulta;
    }
    
    return nuevoID;
}



//---------------------------------------------
// GENERAR DATOS DE PRUEBA (VERSIÓN ARCHIVOS)
//---------------------------------------------


// Función auxiliar para crear y registrar doctor
int crearYRegistrarDoctor(const char* nombre, const char* apellido,
                         const char* cedula, const char* especialidad,
                         int aniosExperiencia, float costoConsulta,
                         const char* telefono, const char* email) {
    // Crear doctor en memoria
    Doctor nuevoDoctor = crearDoctor(nombre, apellido, cedula, especialidad,
                                   aniosExperiencia, costoConsulta,
                                   telefono, email);
    
    // Guardar en archivo
    if (agregarDoctor(nuevoDoctor)) {
        return nuevoDoctor.id; // Retorna el ID asignado
    }
    return -1; // Error
}

// Función auxiliar para crear y registrar paciente
int crearYRegistrarPaciente(const char* nombre, const char* apellido,
                           const char* cedula, int edad, char sexo,
                           const char* tipoSangre, const char* alergias,
                           const char* telefono, const char* direccion,
                           const char* email) {
    // Crear paciente en memoria
    Paciente nuevoPaciente = crearPaciente(nombre, apellido, cedula, edad, sexo,
                                          tipoSangre, alergias, telefono,
                                          direccion, email);
    
    // Guardar en archivo
    if (agregarPaciente(nuevoPaciente)) {
        return nuevoPaciente.id; // Retorna el ID asignado
    }
    return -1; // Error
}

// Función auxiliar para crear y registrar cita
int crearYRegistrarCita(int pacienteID, int doctorID, const char* fecha,
                       const char* hora, const char* motivo) {
    // Crear cita en memoria
    Cita nuevaCita = crearCita(pacienteID, doctorID, fecha, hora, motivo);
    
    // Guardar en archivo
    if (agregarCita(nuevaCita)) {
        // Aquí también necesitas actualizar los arrays de pacientes y doctores
        // Esto se hace automáticamente en registrarCitaCompleta
        // Pero como estamos usando agregarCita directamente, necesitamos hacerlo manualmente
        
        // Obtener el ID de la cita recién creada
        ArchivoHeader headerCitas = leerHeaderCitas();
        int idCita = headerCitas.proximoID - 1;
        
        // Actualizar paciente
        Paciente paciente = buscarPacientePorID(pacienteID);
        if (paciente.id != -1) {
            if (paciente.cantidadCitas < 20) {
                paciente.citasIDs[paciente.cantidadCitas] = idCita;
                paciente.cantidadCitas++;
                actualizarPaciente(paciente);
            }
        }
        
        // Actualizar doctor
        Doctor doctor = buscarDoctorPorID(doctorID);
        if (doctor.id != -1) {
            if (doctor.cantidadCitas < 30) {
                doctor.citasIDs[doctor.cantidadCitas] = idCita;
                doctor.cantidadCitas++;
                actualizarDoctor(doctor);
            }
        }
        
        return idCita;
    }
    return -1; // Error
}



void generarDatosPrueba() {
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 8);
    textoColor(25, 3, "GENERANDO DATOS DE PRUEBA", 14, 1);
    textoColor(10, 5, "Creando datos de prueba... Por favor espere.", 10, 1);
    
    // Arrays para guardar IDs
    int idsDoctores[20];
    int idsPacientes[20];
    int idsCitas[20];
    
    // CREAR 20 DOCTORES
    idsDoctores[0] = crearYRegistrarDoctor("Elena", "Gomez", "DR-01", "Cardiologia", 10, 80.0, "0412-1111111", "carlos@hospital.com");
    idsDoctores[1] = crearYRegistrarDoctor("Luisa", "Valbuena", "DR-02", "Pediatria", 8, 70.0, "0414-2222222", "maria@hospital.com");
    idsDoctores[2] = crearYRegistrarDoctor("German", "Martinez", "DR-03", "Traumatologia", 12, 90.0, "0416-3333333", "juan@hospital.com");
    idsDoctores[3] = crearYRegistrarDoctor("Albis", "Medina", "DR-04", "Dermatologia", 7, 75.0, "0424-4444444", "ana@hospital.com");
    idsDoctores[4] = crearYRegistrarDoctor("Ivanna", "Rodriguez", "DR-05", "Neurologia", 15, 100.0, "0412-5555555", "luis@hospital.com");
    idsDoctores[5] = crearYRegistrarDoctor("Angelica", "Marin", "DR-06", "Ginecologia", 9, 85.0, "0414-6666666", "elena@hospital.com");
    idsDoctores[6] = crearYRegistrarDoctor("Luis", "Govea", "DR-07", "Oftalmologia", 11, 80.0, "0416-7777777", "pedro@hospital.com");
    idsDoctores[7] = crearYRegistrarDoctor("David", "Sanchez", "DR-08", "Psiquiatria", 14, 95.0, "0424-8888888", "laura@hospital.com");
    idsDoctores[8] = crearYRegistrarDoctor("Jose", "Arrieta", "DR-09", "Cirugia", 18, 120.0, "0412-9999999", "miguel@hospital.com");
    idsDoctores[9] = crearYRegistrarDoctor("Yolbir", "Garcia", "DR-10", "Endocrinologia", 6, 70.0, "0414-1010101", "sofia@hospital.com");
    idsDoctores[10] = crearYRegistrarDoctor("Willie", "Linares", "DR-11", "Urologia", 13, 90.0, "0416-1111111", "jorge@hospital.com");
    idsDoctores[11] = crearYRegistrarDoctor("Juan", "Vargas", "DR-12", "Oncologia", 16, 110.0, "0424-1212121", "carmen@hospital.com");
    idsDoctores[12] = crearYRegistrarDoctor("Camila", "Castro", "DR-13", "Ortopedia", 10, 85.0, "0412-1313131", "ricardo@hospital.com");
    idsDoctores[13] = crearYRegistrarDoctor("Antonio", "Chavez", "DR-14", "Pediatria", 5, 65.0, "0414-1414141", "patricia@hospital.com");
    idsDoctores[14] = crearYRegistrarDoctor("Marie", "Mendoza", "DR-15", "Cardiologia", 11, 85.0, "0416-1515151", "fernando@hospital.com");
    idsDoctores[15] = crearYRegistrarDoctor("Ramon", "Colina", "DR-16", "Dermatologia", 8, 75.0, "0424-1616161", "diana@hospital.com");
    idsDoctores[16] = crearYRegistrarDoctor("Carlos", "Machado", "DR-17", "Neurologia", 12, 95.0, "0412-1717171", "roberto@hospital.com");
    idsDoctores[17] = crearYRegistrarDoctor("Lupe", "Barreto", "DR-18", "Ginecologia", 7, 80.0, "0414-1818181", "gabriela@hospital.com");
    idsDoctores[18] = crearYRegistrarDoctor("Sara", "Ardila", "DR-19", "Traumatologia", 9, 85.0, "0416-1919191", "alejandro@hospital.com");
    idsDoctores[19] = crearYRegistrarDoctor("Carmen", "Quintero", "DR-20", "Psiquiatria", 10, 85.0, "0424-2020202", "teresa@hospital.com");
    
    // CREAR 20 PACIENTES
    idsPacientes[0] = crearYRegistrarPaciente("Ghiber", "Linares", "PA-01", 18, 'M', "O+", "Penicilina", "0414-0705771", "Av Principal", "ghiber@gmail.com");
    idsPacientes[1] = crearYRegistrarPaciente("Ana", "Gonzalez", "PA-02", 25, 'F', "A+", "Ninguna", "0412-1111111", "Calle 1", "ana@email.com");
    idsPacientes[2] = crearYRegistrarPaciente("Carlos", "Lopez", "PA-03", 30, 'M', "B+", "Mariscos", "0414-2222222", "Calle 2", "carlos@email.com");
    idsPacientes[3] = crearYRegistrarPaciente("Maria", "Martinez", "PA-04", 22, 'F', "AB+", "Polen", "0416-3333333", "Calle 3", "maria@email.com");
    idsPacientes[4] = crearYRegistrarPaciente("Jose", "Rodriguez", "PA-05", 35, 'M', "O-", "Aspirina", "0424-4444444", "Calle 4", "jose@email.com");
    idsPacientes[5] = crearYRegistrarPaciente("Laura", "Sanchez", "PA-06", 28, 'F', "A-", "Ninguna", "0412-5555555", "Calle 5", "laura@email.com");
    idsPacientes[6] = crearYRegistrarPaciente("Miguel", "Hernandez", "PA-07", 40, 'M', "B-", "Polvo", "0414-6666666", "Calle 6", "miguel@email.com");
    idsPacientes[7] = crearYRegistrarPaciente("Elena", "Diaz", "PA-08", 32, 'F', "AB-", "Acaros", "0416-7777777", "Calle 7", "elena@email.com");
    idsPacientes[8] = crearYRegistrarPaciente("Pedro", "Torres", "PA-09", 45, 'M', "O+", "Latex", "0424-8888888", "Calle 8", "pedro@email.com");
    idsPacientes[9] = crearYRegistrarPaciente("Sofia", "Ramirez", "PA-10", 26, 'F', "A+", "Yodo", "0412-9999999", "Calle 9", "sofia@email.com");
    idsPacientes[10] = crearYRegistrarPaciente("Andres", "Flores", "PA-11", 33, 'M', "B+", "Ninguna", "0414-1010101", "Calle 10", "andres@email.com");
    idsPacientes[11] = crearYRegistrarPaciente("Carmen", "Vargas", "PA-12", 29, 'F', "AB+", "Penicilina", "0416-1111111", "Calle 11", "carmen@email.com");
    idsPacientes[12] = crearYRegistrarPaciente("Ricardo", "Castro", "PA-13", 38, 'M', "O-", "Mariscos", "0424-1212121", "Calle 12", "ricardo@email.com");
    idsPacientes[13] = crearYRegistrarPaciente("Patricia", "Rojas", "PA-14", 24, 'F', "A-", "Polen", "0412-1313131", "Calle 13", "patricia@email.com");
    idsPacientes[14] = crearYRegistrarPaciente("Fernando", "Mendoza", "PA-15", 42, 'M', "B-", "Aspirina", "0414-1414141", "Calle 14", "fernando@email.com");
    idsPacientes[15] = crearYRegistrarPaciente("Diana", "Ortega", "PA-16", 27, 'F', "AB-", "Ninguna", "0416-1515151", "Calle 15", "diana@email.com");
    idsPacientes[16] = crearYRegistrarPaciente("Roberto", "Silva", "PA-17", 36, 'M', "O+", "Polvo", "0424-1616161", "Calle 16", "roberto@email.com");
    idsPacientes[17] = crearYRegistrarPaciente("Gabriela", "Morales", "PA-18", 31, 'F', "A+", "Acaros", "0412-1717171", "Calle 17", "gabriela@email.com");
    idsPacientes[18] = crearYRegistrarPaciente("Alejandro", "Reyes", "PA-19", 39, 'M', "B+", "Latex", "0414-1818181", "Calle 18", "alejandro@email.com");
    idsPacientes[19] = crearYRegistrarPaciente("Teresa", "Perez", "PA-20", 23, 'F', "AB+", "Yodo", "0416-1919191", "Calle 19", "teresa@email.com");
    
    // ASIGNAR PACIENTES A DOCTORES (RELACIONES MÉDICO-PACIENTE)
    // Cardiología - Dr. Elena Gomez
    asignarPacienteADoctorArchivos(idsPacientes[0], idsDoctores[0]);  // Ghiber Linares
    asignarPacienteADoctorArchivos(idsPacientes[4], idsDoctores[0]);  // Jose Rodriguez
    asignarPacienteADoctorArchivos(idsPacientes[14], idsDoctores[0]); // Fernando Mendoza
    
    // Pediatría - Dra. Luisa Valbuena
    asignarPacienteADoctorArchivos(idsPacientes[1], idsDoctores[1]);  // Ana Gonzalez
    asignarPacienteADoctorArchivos(idsPacientes[5], idsDoctores[1]);  // Laura Sanchez
    asignarPacienteADoctorArchivos(idsPacientes[13], idsDoctores[1]); // Patricia Rojas
    
    // Traumatología - Dr. German Martinez
    asignarPacienteADoctorArchivos(idsPacientes[2], idsDoctores[2]);  // Carlos Lopez
    asignarPacienteADoctorArchivos(idsPacientes[8], idsDoctores[2]);  // Pedro Torres
    asignarPacienteADoctorArchivos(idsPacientes[18], idsDoctores[2]); // Alejandro Reyes
    
    // Dermatología - Dr. Albis Medina
    asignarPacienteADoctorArchivos(idsPacientes[3], idsDoctores[3]);  // Maria Martinez
    asignarPacienteADoctorArchivos(idsPacientes[7], idsDoctores[3]);  // Elena Diaz
    asignarPacienteADoctorArchivos(idsPacientes[15], idsDoctores[3]); // Diana Ortega
    
    // Neurología - Dra. Ivanna Rodriguez
    asignarPacienteADoctorArchivos(idsPacientes[6], idsDoctores[4]);  // Miguel Hernandez
    asignarPacienteADoctorArchivos(idsPacientes[10], idsDoctores[4]); // Andres Flores
    asignarPacienteADoctorArchivos(idsPacientes[16], idsDoctores[4]); // Roberto Silva
    
    // Ginecología - Dra. Angelica Marin
    asignarPacienteADoctorArchivos(idsPacientes[9], idsDoctores[5]);  // Sofia Ramirez
    asignarPacienteADoctorArchivos(idsPacientes[11], idsDoctores[5]); // Carmen Vargas
    asignarPacienteADoctorArchivos(idsPacientes[17], idsDoctores[5]); // Gabriela Morales
    
    // Oftalmología - Dr. Luis Govea
    asignarPacienteADoctorArchivos(idsPacientes[12], idsDoctores[6]); // Ricardo Castro
    asignarPacienteADoctorArchivos(idsPacientes[19], idsDoctores[6]); // Teresa Perez
    
    // Psiquiatría - Dr. David Sanchez
    asignarPacienteADoctorArchivos(idsPacientes[0], idsDoctores[7]);  // Ghiber Linares
    asignarPacienteADoctorArchivos(idsPacientes[19], idsDoctores[7]); // Teresa Perez
    
    // CREAR 20 CITAS
    idsCitas[0] = crearYRegistrarCita(idsPacientes[0], idsDoctores[0], "15-01-2025", "09:00", "Consulta cardiologica");
    idsCitas[1] = crearYRegistrarCita(idsPacientes[1], idsDoctores[1], "16-01-2025", "10:00", "Control pediatrico");
    idsCitas[2] = crearYRegistrarCita(idsPacientes[2], idsDoctores[2], "17-01-2025", "11:00", "Dolor rodilla");
    idsCitas[3] = crearYRegistrarCita(idsPacientes[3], idsDoctores[3], "18-01-2025", "12:00", "Erupcion cutanea");
    idsCitas[4] = crearYRegistrarCita(idsPacientes[4], idsDoctores[4], "19-01-2025", "13:00", "Dolor cabeza");
    idsCitas[5] = crearYRegistrarCita(idsPacientes[5], idsDoctores[5], "20-01-2025", "14:00", "Consulta ginecologica");
    idsCitas[6] = crearYRegistrarCita(idsPacientes[6], idsDoctores[6], "21-01-2025", "15:00", "Examen vista");
    idsCitas[7] = crearYRegistrarCita(idsPacientes[7], idsDoctores[7], "22-01-2025", "16:00", "Consulta psicologica");
    idsCitas[8] = crearYRegistrarCita(idsPacientes[8], idsDoctores[8], "23-01-2025", "09:30", "Evaluacion cirugia");
    idsCitas[9] = crearYRegistrarCita(idsPacientes[9], idsDoctores[9], "24-01-2025", "10:30", "Control hormonal");
    idsCitas[10] = crearYRegistrarCita(idsPacientes[10], idsDoctores[10], "25-01-2025", "11:30", "Consulta urologica");
    idsCitas[11] = crearYRegistrarCita(idsPacientes[11], idsDoctores[11], "26-01-2025", "12:30", "Seguimiento oncologico");
    idsCitas[12] = crearYRegistrarCita(idsPacientes[12], idsDoctores[12], "27-01-2025", "13:30", "Dolor espalda");
    idsCitas[13] = crearYRegistrarCita(idsPacientes[13], idsDoctores[13], "28-01-2025", "14:30", "Control nino");
    idsCitas[14] = crearYRegistrarCita(idsPacientes[14], idsDoctores[14], "29-01-2025", "15:30", "Presion alta");
    idsCitas[15] = crearYRegistrarCita(idsPacientes[15], idsDoctores[15], "30-01-2025", "16:30", "Acne");
    idsCitas[16] = crearYRegistrarCita(idsPacientes[16], idsDoctores[16], "01-02-2025", "09:00", "Mareos");
    idsCitas[17] = crearYRegistrarCita(idsPacientes[17], idsDoctores[17], "02-02-2025", "10:00", "Consulta ginecologica");
    idsCitas[18] = crearYRegistrarCita(idsPacientes[18], idsDoctores[18], "03-02-2025", "11:00", "Lesion deportiva");
    idsCitas[19] = crearYRegistrarCita(idsPacientes[19], idsDoctores[19], "04-02-2025", "12:00", "Ansiedad");
    
    // ATENDER 5 CITAS
    atenderCita(idsCitas[0], "Hipertension grado 1", "Dieta baja en sal, ejercicio", "Losartan 50mg");
    atenderCita(idsCitas[1], "Control crecimiento normal", "Seguimiento rutinario", "Multivitaminico");
    atenderCita(idsCitas[2], "Esguince tobillo", "Reposo, hielo, elevacion", "Ibuprofeno 400mg");
    atenderCita(idsCitas[3], "Dermatitis contacto", "Crema hidratante", "Hidrocortisona 1%");
    atenderCita(idsCitas[4], "Migrana tensional", "Manejo estres", "Paracetamol 500mg");
    
    // MENSAJE DE CONFIRMACIÓN ACTUALIZADO
    system("cls");
    system("color 1F");
    dibujarCuadro(2, 3, 75, 12);
    textoColor(25, 3, "DATOS DE PRUEBA GENERADOS", 14, 1);
    textoColor(10, 5, "20 Doctores creados", 10, 1);
    textoColor(10, 6, "20 Pacientes creados", 10, 1);
    textoColor(10, 7, "25 Asignaciones medico-paciente", 10, 1);
    textoColor(10, 8, "20 Citas agendadas", 10, 1);
    textoColor(10, 9, "5 Citas atendidas", 10, 1);
    textoColor(10, 10, "2 Pacientes con doctores multiples", 10, 1);
    textoColor(10, 11, "Presione cualquier tecla...", 14, 1);
    _getch();
}
// MENU PRINCIPAL
//---------------------------------------------
int mostrarMenuPrincipal(Hospital* hospital) {
	
    sincronizarEstadisticas(*hospital);

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
    textoColor(20, 8, "4. Generar Datos de Prueba", 15, 1);
    textoColor(20, 9, "ESC. Salir", 12, 1); // Salir en rojo

    // Línea separadora en gris
    textoColor(10, 11, "================================================", 7, 1);
    
    // ESTADISTICAS con colores
    textoColor(10, 12, "Estadisticas:", 14, 1); // Título en amarillo
    
    textoColor(10, 13, "Pacientes registrados: ", 15, 1);
    textoColor(35, 13, to_string(hospital->totalPacientesRegistrados), 10, 1); // Número en verde
    
    textoColor(10, 14, "Doctores registrados:  ", 15, 1);
    textoColor(35, 14, to_string(hospital->totalDoctoresRegistrados), 10, 1);
    
    textoColor(10, 15, "Citas agendadas:       ", 15, 1);
    textoColor(35, 15, to_string(hospital->totalCitasAgendadas), 10, 1);
    
    // PRÓXIMOS IDS con colores (información adicional útil)
    textoColor(40, 12, "Proximos IDs:", 14, 1);
    
    textoColor(40, 13, "ID Paciente: ", 15, 1);
    textoColor(54, 13, to_string(hospital->siguienteIDPaciente), 11, 1); // Aguamarina
    
    textoColor(40, 14, "ID Doctor:   ", 15, 1);
    textoColor(54, 14, to_string(hospital->siguienteIDDoctor), 11, 1);
    
    textoColor(40, 15, "ID Cita:     ", 15, 1);
    textoColor(54, 15, to_string(hospital->siguienteIDCita), 11, 1);
    
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
        dibujarCuadro(5, 2, 70, 18);
        
        // TÃ­tulo en amarillo
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
                capturarDatosPacienteVisual();
                break;
            case 2:
                buscarPacientePorCedulaVisual(true);
                break;
            case 3:
                buscarPacientesPorNombreVisual();
                break;
            case 4:
                mostrarHistorialMedicoVisual();
                break;
            case 5:
                buscarPacientePorCedulaVisual(false);
                break;
            case 6:
                listarPacientesVisual();
                break;
            case 7:
                eliminarPacienteVisual();
                break;
            case 0:
                return;
            default:
                textoColor(20, 17, "Opcion invalida. Presione una tecla...", 12, 1);
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
                capturarDatosDoctorVisual();
                break;
            case 2:
                buscarDoctorPorIdVisual();
                break;
            case 3:
                buscarDoctoresPorEspecialidadVisual(); 
                break;
            case 4:
                asignarPacienteADoctorVisual();
                break;
            case 5:
                listarPacientesDeDoctorVisual();
                break;
            case 6:
                listarDoctoresVisual();
                break;
            case 7:
                eliminarDoctorVisual();
                break;
            case 0:
                return;
            default:
                textoColor(20, 17, "Opcion invalida. Presione una tecla...", 12, 1);
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
        dibujarCuadro(5, 2, 70, 18);
        
        // TÃ­tulo en amarillo
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
                agendarCitaVisual();
                break;
            case 2:
                cancelarCitaVisual();
                break;
            case 3:
                atenderCitaVisual();
                break;
            case 4:
                obtenerCitasDePacienteVisual();
                break;
            case 5:
                 obtenerCitasDeDoctorVisual();
                break;
            case 6:
                obtenerCitasPorFechaVisual();
                break;
            case 7:
                listarCitasPendientes();
                break;
            case 0:
                return;
            default:
                textoColor(20, 17, "Opcion invalida. Presione una tecla...", 12, 1);
                _getch();
        }
    } while (opcion != 0);
}

//---------------------------------------------
// PROGRAMA PRINCIPAL
//---------------------------------------------
int main() {
	
    // Inicializar hospital con datos reales
    Hospital hospital = cargarHospital();
    
                                           
    
    
    int opcion;
    do {
        opcion = mostrarMenuPrincipal(&hospital); 
        
        if (opcion != 0) 
        switch(opcion) {
            case 1:
                menuPacientes(&hospital);
                break;
            case 2:
                menuDoctores(&hospital);
                break;
            case 3:
                menuCitas(&hospital);
                break;
            case 4:
                generarDatosPrueba();                                           ;
                break;
            default:
                textoColor(20, 16, "Opcion invalida. Presione una tecla...", 12, 1);
                _getch();
        }
    } while (opcion != 0);
    

    return 0;
}

