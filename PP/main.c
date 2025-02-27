#include <stdio.h>
#include <stdbool.h>

// Definicja stanów maszyny stanów
typedef enum {
    STATE_INIT,         //Inicjalizajca
    STATE_SAFE,         //Stan bezpieczny - stan do którego przejść można z każdego momentu jeśli warunki bezpieczeństwa nie są spełnione
    STATE_WAIT,         //Czekanie - wszytskie urządzenia działają poprawnie
    STATE_AUTONOMOUS,   //Ruch autonomiczny
    STATE_JOYSTICK,     //Ruch z joystick'a
} RobotState;

// Struktura reprezentująca warunki wejściowe - wszystkie zmienne które przychodzą do programu
typedef struct {
    // Bezpieczeństwo
    bool power_status_ok;
    bool emergency_stop_active;
    bool safety_scanner_free;
    bool pc_to_controller_comm_ok;
    bool joystick_connected;
    bool plc_to_servo_comm_ok;
    bool safety;

    // Ruch robota
    bool autonomous_allowed;
    bool joystick_allowed;

    // Metody sterowania
    bool joystick_control;
    bool trajectory_tracking;
    bool collision_avoidance;
    bool rgb_d_camera_control;
    bool optitrack_control;
    bool trajectory_loaded;
    bool scanner_measurement_active;

    // Urządzenia - włączone i poprawnie działające urządzenia
    bool actuator_ok;       //trzeba dodac jeszcze kat silownika zaczepu
    bool rgb_d_camera_ok;
    bool sick_sensor_ok;
    bool pc_ok;
    bool router_ok;
    bool servo_ok;


} RobotConditions;


// Funkcja do zarządzania przejściami pomiędzy stanami(ponizej mozliwe przejscia)
//  INIT->SAFE          SAFE->WAIT          WAIT->AUTONOMOUS        AUTONOMOUS->SAFE    AUTONOMOUS->WAIT
//  INIT->WAIT          WAIT->SAFE          WAIT->JOYSTICK          JOYSTICK->SAFE      JOYSTICK->WAIT
RobotState fsm_transition(RobotState current_state, RobotConditions conditions) {
    conditions.safety=(conditions.power_status_ok && conditions.emergency_stop_active && conditions.safety_scanner_free && conditions.pc_to_controller_comm_ok && conditions.joystick_connected && conditions.plc_to_servo_comm_ok && conditions.actuator_ok && conditions.rgb_d_camera_ok && conditions.sick_sensor_ok && conditions.pc_ok &&conditions.router_ok && conditions.servo_ok);
    //safety - zmienna oznaczająca ze jakakolwiek awaria naszla
    switch (current_state) {
        case STATE_INIT:
            if (conditions.safety) {
                return STATE_WAIT;  // Przejście do trybu oczekiwania
            } else {
                return STATE_SAFE;  // Przejście do trybu bezpiecznego
            }

        case STATE_SAFE:
            if (conditions.safety) {
                return STATE_WAIT;  // Powrót do IDLE po rozwiązaniu problemów
            } else {
                return STATE_SAFE;  // Pozostanie w stanie SAFE
            }

        case STATE_WAIT:
            if (!conditions.safety) {
                return STATE_SAFE;  // Problemy z bezpieczeństwem -> SAFE
            } else if (conditions.autonomous_allowed) {
                return STATE_AUTONOMOUS;  // Przejście do trybu autonomicznego
            } else if (conditions.joystick_allowed && conditions.joystick_connected) {
                return STATE_JOYSTICK;  // Przejście do trybu joysticka
            } else {
                return STATE_WAIT;  // Pozostanie w IDLE
            }

        case STATE_AUTONOMOUS:
            if (!conditions.safety) {
                return STATE_SAFE;  // Problemy z bezpieczeństwem -> SAFE
            } else if (!conditions.autonomous_allowed) {
                return STATE_WAIT;  // Powrót do IDLE
            } else {
                return STATE_AUTONOMOUS;  // Pozostanie w trybie autonomicznym
            }

        case STATE_JOYSTICK:
            if (!conditions.safety) {
                return STATE_SAFE;  // Problemy z bezpieczeństwem -> SAFE
            } else if (!conditions.joystick_allowed) {
                return STATE_WAIT;  // Powrót do IDLE
            } else {
                return STATE_JOYSTICK;  // Pozostanie w trybie joysticka
            }

        default:
            return STATE_SAFE;  // Domyślne przejście do trybu SAFE
    }
}

// Funkcja pomocnicza do debugowania (wyświetla nazwę aktualnego stanu)
const char* get_state_name(RobotState state, RobotConditions conditions) {
    switch (state) {
        case STATE_INIT: return "INIT";
        case STATE_SAFE: return "SAFE";
        case STATE_WAIT: return "WAIT";
        case STATE_AUTONOMOUS:                           //jako ze automatyczne sterowanie ma kilka opcji to pokazuje jaka jest aktywna
        if (conditions.trajectory_tracking) {
            return "AUTONOMOUS: trajectory_tracking.";
        }
        if (conditions.collision_avoidance) {
            return "AUTONOMOUS: collision avoidance.";
        }
        if (conditions.rgb_d_camera_control) {
            return "AUTONOMOUS: RGB-D camera control.";
        }
        if (conditions.optitrack_control) {
            return "AUTONOMOUS: optiTrack control.";
        }
        if (conditions.trajectory_loaded) {
            return "AUTONOMOUS: preloaded trajectory.";
        }
        if (conditions.scanner_measurement_active) {
            return "AUTONOMOUS: scanner measurement.";
        }
        // Domyślny przypadek
        return "AUTONOMOUS: No specific method selected.";

        case STATE_JOYSTICK: return "JOYSTICK";
        default: return "UNKNOWN";
    }
}
//funkcja do napisania ok lub zle w zaleznosci od wartosci bool 
const char* Z(bool x){
    
    if(x==1)
     return "ok";
    else
     return "blad";
 }
//funkcja do pokazania zmiennych waznych (safety)
 void View(RobotState state, RobotConditions conditions){
    printf("---------------------------- \n");
    printf("Stan urządzenia: \n");

    printf("Aktuator: ");
    printf(Z(conditions.actuator_ok));
    printf("\n");
    printf("rgb_d_camera_ok: ");
    printf(Z(conditions.rgb_d_camera_ok));
    printf("\n");
    printf("pc: ");
    printf(Z(conditions.pc_ok));
    printf("\n");
    printf("router: ");
    printf(Z(conditions.router_ok));
    printf("\n");
    printf("power_status: ");
    printf(Z(conditions.power_status_ok));
    printf("\n");
    printf("servo: ");
    printf(Z(conditions.servo_ok));    
    printf("\n");
    printf("emergency_stop_active: ");
    printf(Z(conditions.emergency_stop_active));    
    printf("\n");
    printf("safety_scanner_free: ");
    printf(Z(conditions.safety_scanner_free));    
    printf("\n");
    printf("pc_to_controller_comm_ok: ");
    printf(Z(conditions.pc_to_controller_comm_ok));
    printf("\n");
    printf("Podlaczenie Jouystick'a: ");
    printf(Z(conditions.joystick_connected));
    printf("\n");
    printf("Polaczenie pl/servo: ");
    printf(Z(conditions.plc_to_servo_comm_ok));
    printf("\n");
    printf("---------------------------- \n");
    }

  
//Funkcja do stanu STATE_SAFE - pokazuje wszystkie zmienne ktore nie pozwalaja na prace (safety)
void Awaria(RobotState state, RobotConditions conditions){
        if(state==STATE_SAFE){
            printf("-----------------\n");
            printf("Bledy:\n");
            if(conditions.actuator_ok==0){
            printf("Aktuator: ");
            printf(Z(conditions.actuator_ok));
            printf("\n");}

            if(conditions.rgb_d_camera_ok==0){
            printf("rgb_d_camera_ok: ");
            printf(Z(conditions.rgb_d_camera_ok));
            printf("\n");}

            if(conditions.pc_ok==0){
            printf("pc:");
            printf(Z(conditions.pc_ok));
            printf("\n");}

            if(conditions.router_ok==0){
            printf("router: ");
            printf(Z(conditions.router_ok));
            printf("\n");}

            if(conditions.power_status_ok==0){
            printf("power_status: ");
            printf(Z(conditions.power_status_ok));
            printf("\n");}

            if(conditions.servo_ok==0){
            printf("servo: ");
            printf(Z(conditions.servo_ok));    
            printf("\n");}

            if(conditions.emergency_stop_active==0){
            printf("Wylacznik glowny: ");
            printf(Z(conditions.emergency_stop_active));    
            printf("\n");}

            if(conditions.safety_scanner_free==0){
            printf("safety_scanner_free: ");
            printf(Z(conditions.safety_scanner_free));    
            printf("\n");}

            if(conditions.pc_to_controller_comm_ok==0){
            printf("pc_to_controller_comm_ok: ");
            printf(Z(conditions.pc_to_controller_comm_ok));
            printf("\n");}

            if(conditions.joystick_connected==0){
            printf("Podlaczenie Jouystick'a: ");
            printf(Z(conditions.joystick_connected));
            printf("\n");}

            if(conditions.plc_to_servo_comm_ok==0){
            printf("Polaczenie plc/servo: ");
            printf(Z(conditions.plc_to_servo_comm_ok));
            printf("\n");}
            printf("---------------------------- \n");            
        }
}
// Główna pętla programu
int main() {
    // Stan początkowy
    RobotState current_state = STATE_INIT;

    // Przykładowe warunki (zmieniające się dynamicznie w czasie pracy robota)
    RobotConditions conditions = {
        .power_status_ok = true,
        .emergency_stop_active = true,
        .safety_scanner_free = true,
        .pc_to_controller_comm_ok = true,
        .joystick_connected = true,
        .plc_to_servo_comm_ok = true,
        .autonomous_allowed = false,
        .joystick_allowed = false,
        .joystick_control = false,
        .trajectory_tracking = false,
        .collision_avoidance = false,
        .rgb_d_camera_control = false,
        .optitrack_control = false,
        .trajectory_loaded = false,
        .scanner_measurement_active = false,
        .actuator_ok = true,
        .rgb_d_camera_ok = true,
        .sick_sensor_ok = true,
        .pc_ok = true,
        .router_ok = true,
        .servo_ok = true,
        .safety = true
    };

    // Symulacja zmiany warunkow w trakcie pracy - ta symulacja polega na ręcznej zmianie stanu zmiennej np. wyłącznika bezpieczeństwa
    // Symulacja wyglada tak ze w kazdej iteracji programu pokazuje jaki jest stan maszyny na bazie zmiennych podanych w 
    // "RobotConditions conditions" wyzej oraz zmianie ich w trakcie nastepnych iteracji     
    for (int i = 0; i < 20; i++) {
        
        printf("Iteration %d, Current State: %s\n", i, get_state_name(current_state, conditions));
        //View(current_state, conditions);
        Awaria(current_state, conditions);
        if (i == 3) {conditions.emergency_stop_active = false; conditions.actuator_ok = false; }  // Wciśnięcia wyłącznika bezpieczeństwa
        if (i == 5) {conditions.actuator_ok = true; } // Zwolnienie wyłącznika bezpieczeństwa
        if (i == 6) {conditions.emergency_stop_active = true; }
        if (i == 10){conditions.autonomous_allowed = true; conditions.trajectory_tracking=true; }   // Włączenie trybu autonomicznego za pomoca sledzenia
        if (i == 13){conditions.autonomous_allowed = false; conditions.trajectory_tracking=false; }    // Wyłączenie trybu autonomicznego
        if (i == 15) conditions.joystick_allowed = true;        // Włączenie trybu joysticka
        if (i == 16) conditions.power_status_ok = false;        // Awaria zasilania podczas sterowania recznego
        if (i == 17) conditions.power_status_ok = true;         // Awaria zasilania minela
        if (i == 19) conditions.joystick_allowed = false;       // Wyłączenie trybu joysticka
    
        // Aktualizacja stanu na podstawie warunków
        current_state = fsm_transition(current_state, conditions);
    }

    return 0;
}

