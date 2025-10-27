#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>  // Para system()
using namespace std;

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

string leerCampo(int x, int y, int maxlen, bool permitirESC = true) {
    string texto;
    char c;
    while (true) {
        gotoxy(x + texto.size(), y);
        c = _getch();
        
        if (c == 13) break; // Enter - confirmar
        if (c == 27 && permitirESC) { // ESC - cancelar/salir
            texto = "ESC"; // Marcador especial para indicar que se presionó ESC
            break;
        }
        if (c == 8 && !texto.empty()) { // Backspace
            texto.pop_back();
            gotoxy(x + texto.size(), y);
            cout << " ";
            gotoxy(x + texto.size(), y);
        }
        else if (isprint((unsigned char)c) && texto.size() < maxlen) {
            texto.push_back(c);
            cout << c;
        }
    }
    return texto;
}


string leerCampoNumerico(int x, int y, int maxlen, bool permitirESC = true) {
    string texto;
    char c;
    while (true) {
        gotoxy(x + texto.size(), y);
        c = _getch();
        
        if (c == 13) break; // Enter
        if (c == 27 && permitirESC) { // ESC
            texto = "ESC";
            break;
        }
        if (c == 8 && !texto.empty()) { // Backspace
            texto.pop_back();
            gotoxy(x + texto.size(), y);
            cout << " ";
            gotoxy(x + texto.size(), y);
        }
        else if (isdigit((unsigned char)c) && texto.size() < maxlen) {
            texto.push_back(c);
            cout << c;
        }
    }
    return texto;
}

//---------------------------------------------
// DECLARACIONES DE ESTRUCTURAS 
//---------------------------------------------
struct Hospital;
struct Paciente;
struct Doctor;
struct Cita;
struct HistorialMedico;

//---------------------------------------------
// FUNCIONES DE PACIENTES 
//---------------------------------------------
Paciente* crearPaciente(Hospital* hospital, const char* nombre, const char* apellido, 
                        const char* cedula, int edad, char sexo) {
    cout << "\n[FUNCION] crearPaciente - A implementar...";
    _getch();
    return nullptr;
}

Paciente* buscarPacientePorCedula(Hospital* hospital, const char* cedula) {
    cout << "\n[FUNCION] buscarPacientePorCedula - A implementar...";
    _getch();
    return nullptr;
}

Paciente* buscarPacientePorId(Hospital* hospital, int id) {
    cout << "\n[FUNCION] buscarPacientePorId - A implementar...";
    _getch();
    return nullptr;
}

void listarPacientes(Hospital* hospital) {
    cout << "\n[FUNCION] listarPacientes - A implementar...";
    _getch();
}

bool actualizarPaciente(Hospital* hospital, int id) {
    cout << "\n[FUNCION] actualizarPaciente - A implementar...";
    _getch();
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
Doctor* crearDoctor(Hospital* hospital, const char* nombre, const char* apellido,
                   const char* cedula, const char* especialidad, 
                   int aniosExperiencia, float costoConsulta) {
    cout << "\n[FUNCION] crearDoctor - A implementar...";
    _getch();
    return nullptr;
}

Doctor* buscarDoctorPorId(Hospital* hospital, int id) {
    cout << "\n[FUNCION] buscarDoctorPorId - A implementar...";
    _getch();
    return nullptr;
}

void listarDoctores(Hospital* hospital) {
    cout << "\n[FUNCION] listarDoctores - A implementar...";
    _getch();
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
Hospital* inicializarHospital(const char* nombre, int capacidadInicial) {
    cout << "\n[FUNCION] inicializarHospital - A implementar...";
    _getch();
    return nullptr;
}

void destruirHospital(Hospital* hospital) {
    cout << "\n[FUNCION] destruirHospital - A implementar...";
    _getch();
}

//---------------------------------------------
// MENU PRINCIPAL
//---------------------------------------------
int mostrarMenuPrincipal() {
    system("cls");
    dibujarCuadro(5, 2, 70, 20);
    gotoxy(25, 3);
    cout << "SISTEMA DE GESTION HOSPITALARIA";
    
    gotoxy(20, 6); cout << "1. Gestion de Pacientes";
    gotoxy(20, 7); cout << "2. Gestion de Doctores";
    gotoxy(20, 8); cout << "3. Gestion de Citas";
    gotoxy(20, 9); cout << "ESC. Salir";
    
    gotoxy(20, 12); cout << "Seleccione una opcion: ";
    
    string opcion = leerCampoNumerico(42, 12, 1,true);
    
    if (opcion == "ESC") {
         return 0; // Salir inmediatamente
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
        dibujarCuadro(5, 2, 70, 22);
        gotoxy(28, 3);
        cout << "GESTION DE PACIENTES";
        
        gotoxy(20, 6); cout << "1. Registrar nuevo paciente";
        gotoxy(20, 7); cout << "2. Buscar paciente por cedula";
        gotoxy(20, 8); cout << "3. Buscar paciente por nombre";
        gotoxy(20, 9); cout << "4. Ver historial medico completo";
        gotoxy(20, 10); cout << "5. Actualizar datos del paciente";
        gotoxy(20, 11); cout << "6. Listar todos los pacientes";
        gotoxy(20, 12); cout << "7. Eliminar paciente";
        gotoxy(20, 13); cout << "ESC. Volver al menu principal";
        
        gotoxy(20, 16); cout << "Seleccione una opcion: ";
        string opcionStr = leerCampoNumerico(42, 16, 1,true);
        opcion = atoi(opcionStr.c_str());
        
        if (opcionStr == "ESC") {
            return; // Salir inmediatamente
        }        
        
        switch(opcion) {
            case 1:
                crearPaciente(hospital, "Nombre", "Apellido", "Cedula", 30, 'M');
                break;
            case 2:
                buscarPacientePorCedula(hospital, "12345678");
                break;
            case 3:
                // buscarPacientesPorNombre - pendiente
                cout << "\nBuscar por nombre - A implementar...";
                _getch();
                break;
            case 4:
                mostrarHistorialMedico(nullptr);
                break;
            case 5:
                actualizarPaciente(hospital, 1);
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
                gotoxy(20, 18); cout << "Opcion invalida. Presione una tecla...";
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
        dibujarCuadro(5, 2, 70, 22);
        gotoxy(28, 3);
        cout << "GESTION DE DOCTORES";
        
        gotoxy(20, 6); cout << "1. Registrar nuevo doctor";
        gotoxy(20, 7); cout << "2. Buscar doctor por ID";
        gotoxy(20, 8); cout << "3. Buscar doctores por especialidad";
        gotoxy(20, 9); cout << "4. Asignar paciente a doctor";
        gotoxy(20, 10); cout << "5. Ver pacientes asignados a doctor";
        gotoxy(20, 11); cout << "6. Listar todos los doctores";
        gotoxy(20, 12); cout << "7. Eliminar doctor";
        gotoxy(20, 13); cout << "ESC. Volver al menu principal";
        
        gotoxy(20, 16); cout << "Seleccione una opcion: ";
        string opcionStr = leerCampoNumerico(42, 16, 1,true);
        opcion = atoi(opcionStr.c_str());
        
		if (opcionStr == "ESC") {
            return; // Salir inmediatamente
        }            
        
        
        switch(opcion) {
            case 1:
                crearDoctor(hospital, "Dr. Juan", "Perez", "12345", "Cardiologia", 10, 50.0);
                break;
            case 2:
                buscarDoctorPorId(hospital, 1);
                break;
            case 3:
                // buscarDoctoresPorEspecialidad - pendiente
                cout << "\nBuscar por especialidad - A implementar...";
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
                gotoxy(20, 18); cout << "Opcion invalida. Presione una tecla...";
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
        dibujarCuadro(5, 2, 70, 22);
        gotoxy(30, 3);
        cout << "GESTION DE CITAS";
        
        gotoxy(20, 6); cout << "1. Agendar nueva cita";
        gotoxy(20, 7); cout << "2. Cancelar cita";
        gotoxy(20, 8); cout << "3. Atender cita";
        gotoxy(20, 9); cout << "4. Ver citas de un paciente";
        gotoxy(20, 10); cout << "5. Ver citas de un doctor";
        gotoxy(20, 11); cout << "6. Ver citas de una fecha";
        gotoxy(20, 12); cout << "7. Ver citas pendientes";
        gotoxy(20, 13); cout << "ESC. Volver al menu principal";
        
        gotoxy(20, 16); cout << "Seleccione una opcion: ";
        string opcionStr = leerCampoNumerico(42, 16, 1,true);
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
                cout << "\nCitas de paciente - A implementar...";
                _getch();
                break;
            case 5:
                // obtenerCitasDeDoctor - pendiente
                cout << "\nCitas de doctor - A implementar...";
                _getch();
                break;
            case 6:
                // obtenerCitasPorFecha - pendiente
                cout << "\nCitas por fecha - A implementar...";
                _getch();
                break;
            case 7:
                listarCitasPendientes(hospital);
                break;
            case 0:
                return;
            default:
                gotoxy(20, 18); cout << "Opcion invalida. Presione una tecla...";
                _getch();
        }
    } while (opcion != 0);
}

//---------------------------------------------
// PROGRAMA PRINCIPAL
//---------------------------------------------
int main() {
    system("color 1F"); // Fondo azul, texto blanco
    
    // Inicializar hospital (por ahora nullptr)
    Hospital* hospital = nullptr;
    
    int opcion;
    do {
        opcion = mostrarMenuPrincipal();
        
        
        if (opcion!=0) 
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
                cout << "\nOpcion invalida. Presione una tecla...";
                _getch();
        }
    } while (opcion != 0);
    
    // Limpiar memoria
    if(hospital != nullptr) {
        destruirHospital(hospital);
    }
    
    return 0;
}
