#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define HOSTS_FILE "C:\\Windows\\System32\\drivers\\etc\\hosts"
#else
    #include <unistd.h>
    #define HOSTS_FILE "/etc/hosts"
#endif

// Check if the program is running with admin privileges
bool isAdmin() {
#ifdef _WIN32
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
        CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
        FreeSid(AdministratorsGroup);
    }
    return isAdmin;
#else
    return (getuid() == 0);
#endif
}

// Request administrator privileges (Windows only)
void requestAdmin() {
#ifdef _WIN32
    TCHAR exePath[MAX_PATH];
    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path\n";
        exit(1);
    }
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = exePath;
    sei.nShow = SW_SHOWNORMAL;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;

    if (!ShellExecuteEx(&sei)) {
        std::cerr << "Failed to request admin privileges\n";
        exit(1);
    }
    exit(0);
#endif
}

// Check if the IP and domain entry already exists in the hosts file
bool entryExists(const std::string& ip, const std::string& domain) {
    std::ifstream hostsFile(HOSTS_FILE);
    if (!hostsFile) {
        std::cerr << "Error: Unable to open hosts file.\n";
        return false; // Assume it doesn't exist if the file can't be opened
    }

    std::ostringstream entry;
    entry << ip << " " << domain;
    std::string line;

    while (std::getline(hostsFile, line)) {
        if (line.find(entry.str()) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Add an entry to the hosts file
bool addHostEntry(const std::string& ip, const std::string& domain) {
    if (entryExists(ip, domain)) {
        std::cout << "Entry already exists in hosts file.\n";
        return false;
    }

    std::ofstream hostsFile(HOSTS_FILE, std::ios::app);
    if (!hostsFile) {
        std::cerr << "Error: Unable to open hosts file. Try running as administrator, pausing av or using sudo.\n";
        return false;
    }

    hostsFile << ip << " " << domain << "\n";
    std::cout << "Added successfully!\n";
    return true;
}

int main() {
    if (!isAdmin()) {
#ifdef _WIN32
        std::cout << "Requesting administrator privileges...\n";
        requestAdmin();
#else
        std::cerr << "Error: This program must be run as root. Try running with sudo:\n"
            << "sudo " << program_invocation_name << "\n";
        return 1;
#endif
    }

    std::string ip, domain;

    std::cout << "Enter IP: ";
    std::getline(std::cin, ip);
    std::cout << "Enter Domain: ";
    std::getline(std::cin, domain);

    addHostEntry(ip, domain);

    std::cout << "Press any key to continue...";
#ifdef _WIN32
    (void)_getch();
#else
    std::cin.get();
#endif

    return 0;
}