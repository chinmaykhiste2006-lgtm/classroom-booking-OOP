#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
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

// ------------------- Utility functions -------------------

// Convert a time string like "10 AM", "10:30 PM", "12 AM", or "12:00 PM" into minutes since 00:00
int timeStrToMinutes(const string &timeWithPeriod) {
    // Expect formats:
    // "HH AM" or "HH:MM AM" or "H AM" etc.
    // The input could be like "10", "10:30", but our code usually uses "10 AM" (two tokens).
    // Here we handle both a single token with period appended (e.g., "10AM") rarely, but mainly "HH AM".
    string s = timeWithPeriod;
    // Trim
    auto trim = [](string t) {
        size_t a = t.find_first_not_of(" \t\r\n");
        size_t b = t.find_last_not_of(" \t\r\n");
        if (a == string::npos) return string("");
        return t.substr(a, b - a + 1);
    };
    s = trim(s);
    // Find last space to separate period
    // But sometimes input might be "10 AM" (two tokens). We'll expect caller to pass combined.
    // We'll try to split by space:
    string timePart, period;
    size_t pos = s.find_last_of(' ');
    if (pos != string::npos) {
        timePart = s.substr(0, pos);
        period = s.substr(pos + 1);
    } else {
        // fallback: try last two chars as period (AM/PM)
        if (s.size() > 2) {
            period = s.substr(s.size() - 2);
            timePart = s.substr(0, s.size() - 2);
        } else {
            // unknown format, return 0
            return 0;
        }
    }

    // normalize period
    for (auto &c : period) c = toupper(c);

    // parse timePart (HH or HH:MM)
    int hh = 0, mm = 0;
    size_t colon = timePart.find(':');
    if (colon == string::npos) {
        // maybe has stray spaces, trim
        string tp = timePart;
        // remove possible spaces
        tp.erase(remove_if(tp.begin(), tp.end(), ::isspace), tp.end());
        if (tp.empty()) return 0;
        hh = stoi(tp);
        mm = 0;
    } else {
        string hstr = timePart.substr(0, colon);
        string mstr = timePart.substr(colon + 1);
        hh = stoi(hstr);
        mm = stoi(mstr);
    }

    if (period == "PM" && hh != 12) hh += 12;
    if (period == "AM" && hh == 12) hh = 0;

    return hh * 60 + mm;
}

// Parse two tokens (e.g., h and p) into a combined string "h p" and convert
int tokensToMinutes(const string &h, const string &p) {
    string combined = h + " " + p; // e.g., "10 AM" or "10:30 PM"
    return timeStrToMinutes(combined);
}

// Overlap test: returns true if [s1,e1) overlaps [s2,e2)
bool intervalsOverlap(int s1, int e1, int s2, int e2) {
    return (s1 < e2) && (e1 > s2);
}

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
        cout << "User " << name << " Exited.\n";
    }
};

// ------------------- Cleanup Function -------------------
void cleanupOldBookings() {
    // placeholder — no-op
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
                        string day, h1, p1, h2, p2, date;
                        cout << "Enter day (e.g., MONDAY): ";
                        cin >> day;
                        cout << "Enter start time (e.g., 10 AM): ";
                        cin >> h1 >> p1;
                        cout << "Enter end time (e.g., 12 PM): ";
                        cin >> h2 >> p2;
                        cout << "Enter date (YYYY-MM-DD): ";
                        cin >> date;
                        string start = h1 + " " + p1;
                        string end = h2 + " " + p2;
                        showFreeRoomsAtTime(day, start, end, date);

                    } else if (choice == 2) {
                        string room, day, date;
                        cout << "Enter room number: ";
                        cin >> room;
                        cout << "Enter day to check (e.g., MONDAY): ";
                        cin >> day;
                        cout << "Enter date (YYYY-MM-DD): ";
                        cin >> date;
                        showFreeTimesForRoom(room, day, date);

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

    void showFreeRoomsAtTime(const string &day, const string &startTime, const string &endTime, const string &requestedDate) {
        try {
            ifstream normal("normal_timetable.txt");
            if (!normal.is_open()) throw FileException("Could not open normal_timetable.txt");

            vector<string> allRooms;
            vector<string> booked;
            string line;

            // convert requested times to minutes
            int reqStart = timeStrToMinutes(startTime);
            int reqEnd = timeStrToMinutes(endTime);
            if (reqEnd <= reqStart) {
                cout << "Invalid time range entered.\n";
                return;
            }

            // ---- CHECK NORMAL TIMETABLE (recurring weekly entries) ----
            while (getline(normal, line)) {
                string d, room, start, end, br, div, ba, subject;
                stringstream ss(line);
                ss >> d >> room >> start >> end >> br >> div >> ba >> subject;

                if (d == day) {
                    if (find(allRooms.begin(), allRooms.end(), room) == allRooms.end())
                        allRooms.push_back(room);

                    // parse start/end from normal timetable (like "10 AM")
                    int sMin = timeStrToMinutes(start);
                    int eMin = timeStrToMinutes(end);

                    if (intervalsOverlap(sMin, eMin, reqStart, reqEnd))
                        booked.push_back(room);
                }
            }
            normal.close();

            // ---- CHECK BOOKINGS.TXT (date-specific bookings) ----
            ifstream booking("bookings.txt");
            if (!booking.is_open()) {
                // If no bookings file, still print free rooms derived from normal timetable
                // but here we treat missing bookings file as OK
            } else {
                while (getline(booking, line)) {
                    string d, room, start, end, date, br, div, ba, faculty, subject;
                    stringstream ss(line);
                    ss >> d >> room >> start >> end >> date >> br >> div >> ba >> faculty >> subject;

                    if (d == day && date == requestedDate) {
                        if (find(allRooms.begin(), allRooms.end(), room) == allRooms.end())
                            allRooms.push_back(room);

                        int sMin = timeStrToMinutes(start);
                        int eMin = timeStrToMinutes(end);

                        if (intervalsOverlap(sMin, eMin, reqStart, reqEnd))
                            booked.push_back(room);
                    }
                }
                booking.close();
            }

            // remove duplicates in booked (if any)
            sort(booked.begin(), booked.end());
            booked.erase(unique(booked.begin(), booked.end()), booked.end());

            cout << "\nFree rooms on " << day << " (" << requestedDate << ") between " << startTime << " and " << endTime << ":\n";
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
    void showFreeTimesForRoom(const string &room, const string &day, const string &requestedDate) {
        try {
            ifstream normal("normal_timetable.txt");
            if (!normal.is_open()) throw FileException("Could not open normal_timetable.txt");

            vector<pair<int,int>> allIntervals;   // intervals from normal timetable and bookings for this room/day
            vector<pair<int,int>> bookedIntervals; // intervals that are booked (overlap with requested date bookings)

            string line;

            // normal timetable (weekly) entries
            while (getline(normal, line)) {
                string d, r, start, end, br, div, ba, subject;
                stringstream ss(line);
                ss >> d >> r >> start >> end >> br >> div >> ba >> subject;

                if (d == day && r == room) {
                    int sMin = timeStrToMinutes(start);
                    int eMin = timeStrToMinutes(end);
                    allIntervals.emplace_back(sMin, eMin);
                }
            }
            normal.close();

            // bookings on the exact requestedDate for this room
            ifstream booking("bookings.txt");
            if (booking.is_open()) {
                while (getline(booking, line)) {
                    string d, r, start, end, date, br, div, ba, faculty, subject;
                    stringstream ss(line);
                    ss >> d >> r >> start >> end >> date >> br >> div >> ba >> faculty >> subject;

                    if (d == day && r == room && date == requestedDate) {
                        int sMin = timeStrToMinutes(start);
                        int eMin = timeStrToMinutes(end);
                        // a booked interval for this room on that date
                        bookedIntervals.emplace_back(sMin, eMin);

                        // also include it in allIntervals so we can show busy vs free slots from a combined list
                        allIntervals.emplace_back(sMin, eMin);
                    }
                }
                booking.close();
            }

            // sort & unique allIntervals based on start time for displaying
            sort(allIntervals.begin(), allIntervals.end());
            // We will print time slots from allIntervals that are not exactly matched in bookedIntervals
            cout << "\nFree times for room " << room << " on " << day << " (" << requestedDate << "):\n";

            if (allIntervals.empty()) {
                cout << "No standard timetable entries found for this room and day.\n";
                return;
            }

            for (auto &iv : allIntervals) {
                bool isBooked = false;
                for (auto &biv : bookedIntervals) {
                    // if intervals are identical (exact booked interval) or overlap, treat as booked
                    if (iv.first == biv.first && iv.second == biv.second) {
                        isBooked = true;
                        break;
                    }
                    // if the interval iv overlaps with any booked interval, it's not free
                    if (intervalsOverlap(iv.first, iv.second, biv.first, biv.second)) {
                        isBooked = true;
                        break;
                    }
                }
                if (!isBooked) {
                    // print in HH:MM AM/PM format
                    auto printTime = [](int minutes) {
                        int hh = minutes / 60;
                        int mm = minutes % 60;
                        string period = (hh >= 12) ? "PM" : "AM";
                        int displayH = hh % 12;
                        if (displayH == 0) displayH = 12;
                        stringstream out;
                        if (mm == 0) out << displayH << " " << period;
                        else out << displayH << ":" << setw(2) << setfill('0') << mm << " " << period;
                        return out.str();
                    };
                    cout << printTime(iv.first) << " - " << printTime(iv.second) << "\n";
                }
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

                showFreeRoomsAtTime(day, start, end, date);

                char confirm;
                cout << "Do you want to book any of these rooms? (y/n): ";
                cin >> confirm;

                if (confirm == 'y') {
                    string room, subject;
                    cout << "Enter room number to book: ";
                    cin >> room;
                    cout << "Enter subject: ";
                    cin >> subject;

                    // Before writing, we should re-check overlap to avoid race conditions
                    int reqStart = timeStrToMinutes(start);
                    int reqEnd = timeStrToMinutes(end);
                    bool conflict = false;

                    ifstream booking("bookings.txt");
                    string line;
                    if (booking.is_open()) {
                        while (getline(booking, line)) {
                            string d, r, s, e, date2, br, di, ba, fac, sub;
                            stringstream ss(line);
                            ss >> d >> r >> s >> e >> date2 >> br >> di >> ba >> fac >> sub;
                            if (d == day && r == room && date2 == date) {
                                int sMin = timeStrToMinutes(s);
                                int eMin = timeStrToMinutes(e);
                                if (intervalsOverlap(sMin, eMin, reqStart, reqEnd)) {
                                    conflict = true;
                                    break;
                                }
                            }
                        }
                        booking.close();
                    }

                    if (conflict) {
                        cout << "Selected room conflicts with an existing booking on that date/time. Booking cancelled.\n";
                    } else {
                        ofstream out("bookings.txt", ios::app);
                        out << day << " " << room << " " << start << " " << end << " " << date << " "
                            << branch << " " << division << " " << batch << " "
                            << Temp::loggedFaculty << " " << subject << "\n";
                        out.close();
                        cout << "Booking saved!\n";
                    }
                }
                else cout << "Booking cancelled.\n";
            }

            else if (choice == 2) {
                string room, day, date;
                cout << "Enter day: ";
                cin >> day;
                cout << "Enter room number: ";
                cin >> room;
                cout << "Enter date (YYYY-MM-DD): ";
                cin >> date;
                showFreeTimesForRoom(room, day, date);

                char confirm;
                cout << "Do you want to book this room? (y/n): ";
                cin >> confirm;

                if (confirm == 'y') {
                    string h1, p1, h2, p2, subject;
                    cout << "Enter start time (HH AM/PM): ";
                    cin >> h1 >> p1;
                    cout << "Enter end time (HH AM/PM): ";
                    cin >> h2 >> p2;
                    cout << "Enter subject: ";
                    cin >> subject;

                    string start = h1 + " " + p1;
                    string end = h2 + " " + p2;

                    int reqStart = timeStrToMinutes(start);
                    int reqEnd = timeStrToMinutes(end);
                    bool conflict = false;

                    ifstream booking("bookings.txt");
                    string line;
                    if (booking.is_open()) {
                        while (getline(booking, line)) {
                            string d, r, s, e, date2, br, di, ba, fac, sub;
                            stringstream ss(line);
                            ss >> d >> r >> s >> e >> date2 >> br >> di >> ba >> fac >> sub;
                            if (d == day && r == room && date2 == date) {
                                int sMin = timeStrToMinutes(s);
                                int eMin = timeStrToMinutes(e);
                                if (intervalsOverlap(sMin, eMin, reqStart, reqEnd)) {
                                    conflict = true;
                                    break;
                                }
                            }
                        }
                        booking.close();
                    }

                    if (conflict) {
                        cout << "Selected time conflicts with an existing booking on that date. Booking cancelled.\n";
                    } else {
                        ofstream out("bookings.txt", ios::app);
                        out << day << " " << room << " " << start << " " << end << " " << date << " "
                            << branch << " " << division << " " << batch << " "
                            << Temp::loggedFaculty << " " << subject << "\n";
                        out.close();
                        cout << "Booking saved!\n";
                    }
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
