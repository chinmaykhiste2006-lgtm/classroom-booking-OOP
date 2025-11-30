#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <ctime>
#include <sstream>
#include <iomanip>
using namespace std;

// ------------------- Custom Exceptions -------------------
class InvalidChoiceException {
    string msg;
public:
    InvalidChoiceException(string m) { msg = m; }
    string what() { return msg; }
};

class LoginFailedException {
    string msg;
public:
    LoginFailedException(string m) { msg = m; }
    string what() { return msg; }
};

class FileException {
    string msg;
public:
    FileException(string m) { msg = m; }
    string what() { return msg; }
};

class RoomNotFoundException {
    string msg;
public:
    RoomNotFoundException(string m) { msg = m; }
    string what() { return msg; }
};

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
    // We NO longer delete old bookings because AM/PM
    // cannot be compared to a 24-hour system
    return;
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
    void menu() {
        try {
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
                        string day, h1, p1, h2, p2;
                        cout << "Enter day (e.g., MONDAY): ";
                        cin >> day;
                        cout << "Enter start time (e.g., 10 AM): ";
                        cin >> h1 >> p1;
                        cout << "Enter end time (e.g., 12 PM): ";
                        cin >> h2 >> p2;
                        string start = h1 + " " + p1;
                        string end = h2 + " " + p2;
                        showFreeRoomsAtTime(day, start, end);

                    } else if (choice == 2) {
                        string room, day;
                        cout << "Enter room number: ";
                        cin >> room;
                        cout << "Enter day to check: ";
                        cin >> day;
                        showFreeTimesForRoom(room, day);

                    } else if (choice == 3) {
                        bookSlot();

                    } else if (choice == 4) {
                        removeBooking();

                    } else if (choice == 5) {
                        cout << "Exiting faculty menu...\n";

                    } else {
                        throw InvalidChoiceException("Invalid choice entered.");
                    }
                } while (choice != 5);

            } else if (role == "student") {
                int choice = 0;
                do {
                    cout << "\n1. View Timetable\n2. Exit\nEnter choice: ";
                    cin >> choice;

                    if (choice == 1) showStudentTimetable();
                    else if (choice == 2) cout << "Exiting student menu...\n";
                    else throw InvalidChoiceException("Invalid choice entered.");

                } while (choice != 2);
            }
        }
        catch (InvalidChoiceException e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    // ------------------- Show Free Rooms -------------------
    void showFreeRoomsAtTime(string day, string startTime, string endTime) {
        try {
            ifstream normal("normal_timetable.txt");
            if (!normal.is_open()) throw FileException("Could not open normal_timetable.txt");

            vector<string> allRooms;
            vector<string> booked;
            string line;

            // ---- CHECK NORMAL TIMETABLE ----
            while (getline(normal, line)) {
                string d, room, start, end, branch, div, batch, subject;
                stringstream ss(line);
                ss >> d >> room >> start >> end >> branch >> div >> batch >> subject;

                if (d == day) {
                    if (find(allRooms.begin(), allRooms.end(), room) == allRooms.end())
                        allRooms.push_back(room);

                    if (start == startTime || end == endTime)
                        booked.push_back(room);
                }
            }
            normal.close();

            // ---- CHECK BOOKINGS.TXT ----
            ifstream booking("bookings.txt");
            if (!booking.is_open()) throw FileException("Could not open bookings.txt");

            while (getline(booking, line)) {
                string d, room, start, end, date, b, div, ba, faculty, subject;
                stringstream ss(line);
                ss >> d >> room >> start >> end >> date >> b >> div >> ba >> faculty >> subject;

                if (d == day) {
                    if (find(allRooms.begin(), allRooms.end(), room) == allRooms.end())
                        allRooms.push_back(room);

                    if (start == startTime || end == endTime)
                        booked.push_back(room);
                }
            }
            booking.close();

            cout << "\nFree rooms on " << day << " between " << startTime << " and " << endTime << ":\n";
            int count = 0;
            for (auto &r : allRooms) {
                if (find(booked.begin(), booked.end(), r) == booked.end()) {
                    cout << r << " ";
                    count++;
                }
            }
            if (count == 0) throw RoomNotFoundException("No free rooms found for given time.");
            cout << "\n";

        }
        catch (FileException e) { cout << "Error: " << e.what() << endl; }
        catch (RoomNotFoundException e) { cout << "Error: " << e.what() << endl; }
    }

    // ------------------- Show Free Times for Room -------------------
    void showFreeTimesForRoom(string room, string day) {
        try {
            ifstream normal("normal_timetable.txt");
            if (!normal.is_open()) throw FileException("Could not open normal_timetable.txt");

            vector<string> allStart, allEnd, bookedStart, bookedEnd;
            string line;

            while (getline(normal, line)) {
                string d, r, start, end, branch, div, batch, subject;
                stringstream ss(line);
                ss >> d >> r >> start >> end >> branch >> div >> batch >> subject;

                if (d == day) {
                    allStart.push_back(start);
                    allEnd.push_back(end);

                    if (r == room) {
                        bookedStart.push_back(start);
                        bookedEnd.push_back(end);
                    }
                }
            }
            normal.close();

            ifstream booking("bookings.txt");
            while (getline(booking, line)) {
                string d, r, start, end, date, b, div, ba, faculty, subject;
                stringstream ss(line);
                ss >> d >> r >> start >> end >> date >> b >> div >> ba >> faculty >> subject;

                if (d == day) {
                    allStart.push_back(start);
                    allEnd.push_back(end);

                    if (r == room) {
                        bookedStart.push_back(start);
                        bookedEnd.push_back(end);
                    }
                }
            }
            booking.close();

            cout << "\nFree times for room " << room << " on " << day << ":\n";
            for (int i = 0; i < allStart.size(); i++) {
                bool isBooked = false;
                for (int j = 0; j < bookedStart.size(); j++) {
                    if (allStart[i] == bookedStart[j] && allEnd[i] == bookedEnd[j])
                        isBooked = true;
                }
                if (!isBooked)
                    cout << allStart[i] << " - " << allEnd[i] << "\n";
            }
        }
        catch (FileException e) { cout << "Error: " << e.what() << endl; }
    }

    // ------------------- Book Slot -------------------
    void bookSlot() {
        try {
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
                string day, h1, p1, h2, p2, date;
                cout << "Enter day: ";
                cin >> day;
                cout << "Enter start time (HH AM/PM): ";
                cin >> h1 >> p1;
                cout << "Enter end time (HH AM/PM): ";
                cin >> h2 >> p2;
                cout << "Enter date (YYYY-MM-DD): ";
                cin >> date;

                string start = h1 + " " + p1;
                string end = h2 + " " + p2;

                showFreeRoomsAtTime(day, start, end);

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
                    out << day << " " << room << " " << start << " " << end << " " << date << " "
                        << branch << " " << division << " " << batch << " "
                        << Temp::loggedFaculty << " " << subject << "\n";
                    out.close();
                    cout << "Booking saved!\n";
                }
                else cout << "Booking cancelled.\n";
            }

            else if (choice == 2) {
                string room, day;
                cout << "Enter day: ";
                cin >> day;
                cout << "Enter room number: ";
                cin >> room;
                showFreeTimesForRoom(room, day);

                char confirm;
                cout << "Do you want to book this room? (y/n): ";
                cin >> confirm;

                if (confirm == 'y') {
                    string h1, p1, h2, p2, date, subject;
                    cout << "Enter start time (HH AM/PM): ";
                    cin >> h1 >> p1;
                    cout << "Enter end time (HH AM/PM): ";
                    cin >> h2 >> p2;
                    cout << "Enter date (YYYY-MM-DD): ";
                    cin >> date;
                    cout << "Enter subject: ";
                    cin >> subject;

                    string start = h1 + " " + p1;
                    string end = h2 + " " + p2;

                    ofstream out("bookings.txt", ios::app);
                    out << day << " " << room << " " << start << " " << end << " " << date << " "
                        << branch << " " << division << " " << batch << " "
                        << Temp::loggedFaculty << " " << subject << "\n";
                    out.close();
                    cout << "Booking saved!\n";
                }
                else cout << "Booking cancelled.\n";
            }
        }
        catch (InvalidChoiceException e) { cout << "Error: " << e.what() << endl; }
        catch (FileException e) { cout << "Error: " << e.what() << endl; }
    }

    // ------------------- Remove Booking -------------------
    void removeBooking() {
        try {
            ifstream in("bookings.txt");
            if (!in.is_open()) throw FileException("No bookings file found.");

            vector<string> lines;
            vector<string> ownBookings;
            string line;
            while (getline(in, line)) {
                lines.push_back(line);
                if (line.find(Temp::loggedFaculty) != string::npos)
                    ownBookings.push_back(line);
            }
            in.close();

            if (ownBookings.empty()) throw RoomNotFoundException("You have no active bookings.");

            cout << "\nYour Bookings:\n";
            for (int i = 0; i < ownBookings.size(); i++)
                cout << i + 1 << ". " << ownBookings[i] << "\n";

            cout << "Enter the number of the booking to remove (0 to cancel): ";
            int idx;
            cin >> idx;
            if (idx <= 0 || idx > ownBookings.size())
                throw InvalidChoiceException("Invalid booking number.");

            string toRemove = ownBookings[idx - 1];
            ofstream out("bookings.txt", ios::trunc);
            for (auto &l : lines)
                if (l != toRemove)
                    out << l << "\n";
            out.close();

            cout << "Booking removed successfully!\n";
        }
        catch (FileException e) { cout << "Error: " << e.what() << endl; }
        catch (InvalidChoiceException e) { cout << "Error: " << e.what() << endl; }
        catch (RoomNotFoundException e) { cout << "Error: " << e.what() << endl; }
    }

    // ------------------- Student Timetable -------------------
    void showStudentTimetable() {
        try {
            string line;
            cout << "\nTimetable for " << name << " (" << branch << " " << division << " " << batch << "):\n";

            ifstream normal("normal_timetable.txt");
            if (!normal.is_open()) throw FileException("Could not open normal_timetable.txt.");
            while (getline(normal, line)) {
                if (line.find(branch + " " + division + " " + batch) != string::npos)
                    cout << line << "\n";
            }
            normal.close();

            ifstream booking("bookings.txt");
            if (!booking.is_open()) throw FileException("Could not open bookings.txt.");
            while (getline(booking, line)) {
                if (line.find(branch + " " + division + " " + batch) != string::npos)
                    cout << line << " (Booked)\n";
            }
            booking.close();
        }
        catch (FileException e) { cout << "Error: " << e.what() << endl; }
    }
};

// ------------------- Faculty Signup -------------------
void facultySignup() {
    try {
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
        if (!out.is_open()) throw FileException("Unable to open faculty.txt for writing.");
        out << name << "|" << subjects << "|" << userID << "|" << password << "\n";
        out.close();
        cout << "Faculty registered successfully!\n";
    }
    catch (FileException e) {
        cout << "Error: " << e.what() << endl;
    }
}

// ------------------- Faculty Login -------------------
bool facultyLogin() {
    cleanupOldBookings();

    try {
        string userID, password;
        cout << "Enter userID: ";
        cin >> userID;
        cout << "Enter password: ";
        cin >> password;

        ifstream in("faculty.txt");
        if (!in.is_open()) throw FileException("faculty.txt missing.");

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
        throw LoginFailedException("Invalid credentials.");
    }
    catch (FileException e) { cout << "Error: " << e.what() << endl; }
    catch (LoginFailedException e) { cout << "Error: " << e.what() << endl; }
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
    cleanupOldBookings();

    try {
        int choice = 0;
        do {
            cout << "\n--- TIMETABLE MANAGEMENT SYSTEM ---\n";
            cout << "1. Faculty Signup\n2. Faculty Login\n3. Student Login\n4. Exit\nEnter choice: ";
            cin >> choice;

            if (choice == 1) facultySignup();
            else if (choice == 2) facultyLogin();
else if (choice == 3) studentLogin();
            else if (choice == 4) cout << "Goodbye!\n";
            else throw InvalidChoiceException("Invalid main menu choice.");

        } while (choice != 4);
    }
    catch (InvalidChoiceException e) {
        cout << "Error: " << e.what() << endl;
    }
    return 0;
}
