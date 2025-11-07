#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <ctime>
#include <sstream>
#include <iomanip>
using namespace std;

// ------------------- Temp Class -------------------
class Temp {
public:
    static string loggedFaculty;
};
string Temp::loggedFaculty = "";

// ------------------- Base User Class -------------------
class User {
protected:
    string name;
    string role;

public:
    User(string n, string r) {
        name = n;
        role = r;
    }

    virtual void menu() = 0;

    virtual ~User() {
        cout << "User " << name << " destroyed.\n";
    }
};

// ------------------- Cleanup Function -------------------
void cleanupOldBookings() {
    ifstream in("bookings.txt");
    if (!in.is_open()) return;

    vector<string> validLines;
    string line;
    time_t now = time(0);

    while (getline(in, line)) {
        stringstream ss(line);
        string room, start, end, date, branch, division, batch, faculty, subject;
        ss >> room >> start >> end >> date >> branch >> division >> batch >> faculty >> subject;

        if (date.empty() || end.empty()) {
            validLines.push_back(line);
            continue;
        }

        tm bookingEnd = {};
        string dateTime = date + " " + end;
        istringstream dt(dateTime);
        dt >> get_time(&bookingEnd, "%Y-%m-%d %H:%M");

        if (dt.fail()) {
            validLines.push_back(line); // keep malformed lines
            continue;
        }

        time_t endTime = mktime(&bookingEnd);
        if (endTime > now)
            validLines.push_back(line);
    }
    in.close();

    ofstream out("bookings.txt", ios::trunc);
    for (const auto& l : validLines)
        out << l << "\n";
    out.close();
}

// ------------------- TimetableManager Class -------------------
class TimetableManager : public User {
    string division;
    string batch;
    string branch;

public:
    TimetableManager(string n) : User(n, "faculty") {}
    TimetableManager(string n, string d, string b, string br) : User(n, "student") {
        division = d;
        batch = b;
        branch = br;
    }

    // ------------------- Menu -------------------
    void menu() override {
        if (role == "faculty") {
            int choice = 0;
            do {
                cout << "\n1. Check free rooms at time";
                cout << "\n2. Check free times for room";
                cout << "\n3. Book a slot";
                cout << "\n4. Remove a booking";
                cout << "\n5. Exit";
                cout << "\nEnter choice: ";
                cin >> choice;

                if (choice == 1) {
                    string start, end;
                    cout << "Enter start time: ";
                    cin >> start;
                    cout << "Enter end time: ";
                    cin >> end;
                    showFreeRoomsAtTime(start, end);
                } else if (choice == 2) {
                    string room;
                    cout << "Enter room number: ";
                    cin >> room;
                    showFreeTimesForRoom(room);
                } else if (choice == 3) {
                    bookSlot();
                } else if (choice == 4) {
                    removeBooking();
                } else if (choice == 5) {
                    cout << "Exiting faculty menu...\n";
                } else {
                    cout << "Invalid choice.\n";
                }
            } while (choice != 5);
        } else if (role == "student") {
            int choice = 0;
            do {
                cout << "\n1. View Timetable\n2. Exit\nEnter choice: ";
                cin >> choice;

                if (choice == 1) showStudentTimetable();
                else if (choice == 2) cout << "Exiting student menu...\n";
                else cout << "Invalid choice.\n";
            } while (choice != 2);
        }
    }

    // ------------------- Show Free Rooms -------------------
    void showFreeRoomsAtTime(string startTime, string endTime) {
        vector<string> allRooms;
        vector<string> booked;
        string line;

        // Normal timetable
        ifstream normal("normal_timetable.txt");
        while (getline(normal, line)) {
            int pos1 = line.find(' ');
            int pos2 = line.find(' ', pos1 + 1);
            int pos3 = line.find(' ', pos2 + 1);

            string room = line.substr(0, pos1);
            string start = line.substr(pos1 + 1, pos2 - pos1 - 1);
            string end = line.substr(pos2 + 1, pos3 - pos2 - 1);

            if (find(allRooms.begin(), allRooms.end(), room) == allRooms.end())
                allRooms.push_back(room);

            if ((start == startTime) || (end == endTime) || (start == endTime) || (end == startTime))
                booked.push_back(room);
        }
        normal.close();

        // Bookings
        ifstream booking("bookings.txt");
        while (getline(booking, line)) {
            stringstream ss(line);
            string room, start, end, date, branch, division, batch, faculty, subject;
            ss >> room >> start >> end >> date >> branch >> division >> batch >> faculty >> subject;

            if (find(allRooms.begin(), allRooms.end(), room) == allRooms.end())
                allRooms.push_back(room);

            if ((start == startTime) || (end == endTime) || (start == endTime) || (end == startTime))
                booked.push_back(room);
        }
        booking.close();

        cout << "\nFree rooms between " << startTime << " and " << endTime << ":\n";
        for (const auto& r : allRooms) {
            if (find(booked.begin(), booked.end(), r) == booked.end())
                cout << r << " ";
        }
        cout << "\n";
    }

    // ------------------- Show Free Times for Room -------------------
    void showFreeTimesForRoom(string room) {
        vector<string> allStart, allEnd, bookedStart, bookedEnd;
        string line;

        ifstream normal("normal_timetable.txt");
        while (getline(normal, line)) {
            int pos1 = line.find(' ');
            int pos2 = line.find(' ', pos1 + 1);
            int pos3 = line.find(' ', pos2 + 1);

            string r = line.substr(0, pos1);
            string start = line.substr(pos1 + 1, pos2 - pos1 - 1);
            string end = line.substr(pos2 + 1, pos3 - pos2 - 1);

            if (find(allStart.begin(), allStart.end(), start) == allStart.end())
                allStart.push_back(start);
            allEnd.push_back(end);

            if (r == room) {
                bookedStart.push_back(start);
                bookedEnd.push_back(end);
            }
        }
        normal.close();

        ifstream booking("bookings.txt");
        while (getline(booking, line)) {
            stringstream ss(line);
            string r, start, end, date, branch, division, batch, faculty, subject;
            ss >> r >> start >> end >> date >> branch >> division >> batch >> faculty >> subject;

            if (find(allStart.begin(), allStart.end(), start) == allStart.end())
                allStart.push_back(start);
            allEnd.push_back(end);

            if (r == room) {
                bookedStart.push_back(start);
                bookedEnd.push_back(end);
            }
        }
        booking.close();

        cout << "\nFree times for room " << room << ":\n";
        for (int i = 0; i < allStart.size(); i++) {
            bool isBooked = false;
            for (int j = 0; j < bookedStart.size(); j++)
                if (allStart[i] == bookedStart[j] && allEnd[i] == bookedEnd[j]) {
                    isBooked = true;
                    break;
                }
            if (!isBooked)
                cout << allStart[i] << " - " << allEnd[i] << "\n";
        }
        cout << "\n";
    }

    // ------------------- Book Slot -------------------
    void bookSlot() {
        int choice;
        cout << "\nBook Slot Options:\n";
        cout << "1. Enter Start and End Time (to find free rooms)\n";
        cout << "2. Enter Room Number (to find free times)\n";
        cout << "Enter choice: ";
        cin >> choice;

        string branch, division, batch;
        cout << "Enter branch: ";
        cin >> branch;
        cout << "Enter division: ";
        cin >> division;
        cout << "Enter batch: ";
        cin >> batch;

        if (choice == 1) {
            string start, end, date;
            cout << "Enter start time (HH:MM): ";
            cin >> start;
            cout << "Enter end time (HH:MM): ";
            cin >> end;
            cout << "Enter date (YYYY-MM-DD): ";
            cin >> date;

            showFreeRoomsAtTime(start, end);

            char confirm;
            cout << "Do you want to book any of these rooms? (y/n): ";
            cin >> confirm;

            if (confirm == 'y') {
                string room, subject;
                cout << "Enter room number to book: ";
                cin >> room;
                cout << "Enter subject: ";
                cin >> subject;

                ofstream out("bookings.txt", ios::app);
                out << room << " " << start << " " << end << " " << date << " "
                    << branch << " " << division << " " << batch << " "
                    << Temp::loggedFaculty << " " << subject << "\n";
                out.close();
                cout << "Booking saved!\n";
            } else cout << "Booking cancelled.\n";
        }
        else if (choice == 2) {
            string room;
            cout << "Enter room number: ";
            cin >> room;
            showFreeTimesForRoom(room);

            char confirm;
            cout << "Do you want to book this room? (y/n): ";
            cin >> confirm;

            if (confirm == 'y') {
                string start, end, date, subject;
                cout << "Enter start time (HH:MM): ";
                cin >> start;
                cout << "Enter end time (HH:MM): ";
                cin >> end;
                cout << "Enter date (YYYY-MM-DD): ";
                cin >> date;
                cout << "Enter subject: ";
                cin >> subject;

                ofstream out("bookings.txt", ios::app);
                out << room << " " << start << " " << end << " " << date << " "
                    << branch << " " << division << " " << batch << " "
                    << Temp::loggedFaculty << " " << subject << "\n";
                out.close();
                cout << "Booking saved!\n";
            } else cout << "Booking cancelled.\n";
        } else cout << "Invalid choice.\n";
    }

    // ------------------- Remove Booking -------------------
    void removeBooking() {
        ifstream in("bookings.txt");
        if (!in.is_open()) {
            cout << "No bookings found.\n";
            return;
        }

        vector<string> lines;
        vector<string> ownBookings;
        string line;
        while (getline(in, line)) {
            lines.push_back(line);
            if (line.find(Temp::loggedFaculty) != string::npos)
                ownBookings.push_back(line);
        }
        in.close();

        if (ownBookings.empty()) {
            cout << "You have no active bookings.\n";
            return;
        }

        cout << "\nYour Bookings:\n";
        for (int i = 0; i < ownBookings.size(); i++)
            cout << i + 1 << ". " << ownBookings[i] << "\n";

        cout << "Enter the number of the booking to remove (0 to cancel): ";
        int idx;
        cin >> idx;
        if (idx <= 0 || idx > ownBookings.size()) {
            cout << "Cancelled.\n";
            return;
        }

        string toRemove = ownBookings[idx - 1];
        ofstream out("bookings.txt", ios::trunc);
        for (auto& l : lines)
            if (l != toRemove)
                out << l << "\n";
        out.close();

        cout << "Booking removed successfully!\n";
    }

    // ------------------- Student Timetable -------------------
    void showStudentTimetable() {
        string line;
        cout << "\nTimetable for " << name << " (" << branch << " " << division << " " << batch << "):\n";

        ifstream normal("normal_timetable.txt");
        while (getline(normal, line)) {
            if (line.find(branch + division + batch) != string::npos)
                cout << line << "\n";
        }
        normal.close();

        ifstream booking("bookings.txt");
        while (getline(booking, line)) {
            if (line.find(branch + " " + division + " " + batch) != string::npos)
                cout << line << " (Booked)\n";
        }
        booking.close();
    }
};

// ------------------- Faculty Signup -------------------
void facultySignup() {
    string name, subjects, userID, password;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Enter faculty name: ";
    getline(cin, name);
    cout << "Enter subjects (comma separated): ";
    getline(cin, subjects);
    cout << "Enter userID: ";
    getline(cin, userID);
    cout << "Enter password: ";
    getline(cin, password);

    ofstream out("faculty.txt", ios::app);
    out << name << "|" << subjects << "|" << userID << "|" << password << "\n";
    out.close();
    cout << "Faculty registered successfully!\n";
}

// ------------------- Faculty Login -------------------
bool facultyLogin() {
    cleanupOldBookings(); // Clean old data on login

    string userID, password;
    cout << "Enter userID: ";
    cin >> userID;
    cout << "Enter password: ";
    cin >> password;

    ifstream in("faculty.txt");
    string line;
    while (getline(in, line)) {
        int pos1 = line.find('|');
        int pos2 = line.find('|', pos1 + 1);
        int pos3 = line.find('|', pos2 + 1);

        string name = line.substr(0, pos1);
        string subjects = line.substr(pos1 + 1, pos2 - pos1 - 1);
        string id = line.substr(pos2 + 1, pos3 - pos2 - 1);
        string pass = line.substr(pos3 + 1);

        if (id == userID && pass == password) {
            cout << "Login successful! Welcome, " << name << ".\n";
            Temp::loggedFaculty = name;
            TimetableManager t(name);
            t.menu();
            return true;
        }
    }
    cout << "Invalid credentials!\n";
    return false;
}

// ------------------- Student Login -------------------
void studentLogin() {
    string name, branch, division, batch;
    cout << "Enter student name: ";
    cin >> name;
    cout << "Enter branch: ";
    cin >> branch;
    cout << "Enter division: ";
    cin >> division;
    cout << "Enter batch: ";
    cin >> batch;

    TimetableManager t(name, division, batch, branch);
    t.menu();
}

// ------------------- Main -------------------
int main() {
    cleanupOldBookings(); // Clean expired bookings on start

    int choice = 0;
    do {
        cout << "\n--- TIMETABLE MANAGEMENT SYSTEM ---\n";
        cout << "1. Faculty Signup\n2. Faculty Login\n3. Student Login\n4. Exit\nEnter choice: ";
        cin >> choice;

        if (choice == 1) facultySignup();
        else if (choice == 2) facultyLogin();
        else if (choice == 3) studentLogin();
        else if (choice == 4) cout << "Goodbye!\n";
        else cout << "Invalid choice.\n";

    } while (choice != 4);

    return 0;
}
