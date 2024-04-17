#include <iostream>
#include <string>

#include "SQLServerConn.h"
#include "DBConn.h"
#include "CustomerManager.h"
#include "SupplierManager.h"
#include "SupplierNameManager.h"

#include "RetailApp.h"

int main() {
    try {

        std::string connectionString = "DRIVER={SQL Server};SERVER=KN\\SQLEXPRESS;Trusted_Connection=yes;";
        std::string dbName = "sample_store";
        std::string customerTableName = "Customers";
        std::string supplierTableName = "Suppliers";
        std::string supplierNameTableName = "Supplier_Names";


        // Connect to SQL Server instance on 
        SQLServerConn connector;
        connector.connect(connectionString);

        // Connect to database 
        DBConn dbConn(connector.getHDBC());
        if (!dbConn.dbExists(dbName)) {
            dbConn.createDatabase(dbName);
        }
        dbConn.useDatabase(dbName);


        // Create manager for 'customer'. Create table if it doesn't already exist
        CustomerManager customerManager(dbConn, customerTableName);
        if (!dbConn.tableExists(customerTableName)) {
            customerManager.initTable();
        }

        /*
        - Create managers for the SupplierName and Supplier.

        NOTE: Then after this you must initialize the Supplier table first, adn then the
        supplier name table. This is because the latter references the former, so we must make sure
        the former exists before the latter.
        */
        SupplierNameManager supplierNameManager(dbConn, supplierNameTableName, supplierTableName);
        SupplierManager supplierManager(dbConn, supplierTableName, supplierNameManager);

        if (!dbConn.tableExists(supplierTableName)) {
            supplierManager.initTable();
        }

        if (!dbConn.tableExists(supplierNameTableName)) {
            supplierNameManager.initTable();
        }

        

        RetailApp myStore(customerManager, supplierManager, supplierNameManager);
        int choice;

        do {
            // Display main menu and prompt input
            std::cout << "Main Menu: " << std::endl;
            std::cout << "1. Customers" << std::endl;
            std::cout << "2. Suppliers" << std::endl;
            std::cout << "3. Quit" << std::endl;
            std::cout << "Please enter a number to continue: ";
            std::cin >> choice;

            if (std::cin.fail()) {
                std::cout << "Invalid input. Please enter a number!" << std::endl;
                std::cin.clear(); // Clear the error flag
                std::cin.ignore(64, '\n'); // Clear input buffer up to 64 characters or until newline is encountered
                continue; // Restart the loop
            }

            switch (choice) {
            case 1:
                myStore.handleCustomerMenu();
                break;
            case 2:
                myStore.handleSupplierMenu();
                break;
            case 3:
                std::cout << "Exiting Program!" << std::endl;
                break;
            default:
                std::cout << "Invalid choice. Please enter a number between 1 and 3." << std::endl;
            }

        } while (choice != 3);
        

        // Disconnect from SQL Server
        connector.disconnect();
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }
    return 0; // Don't forget to return a value from main
}
