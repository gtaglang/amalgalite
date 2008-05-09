#include "amalgalite3.h"
/* 
 * vim: shiftwidth=4 
 */ 

/**
 * :call-seq:
 *    Amalagliate::SQLite3::Database.open( filename, flags = READWRITE | CREATE ) -> Database
 *
 * Create a new SQLite3 database with a UTF-8 encoding.
 *
 */ 
VALUE am_sqlite3_database_open(int argc, VALUE *argv, VALUE class)
{
    VALUE  self = am_sqlite3_database_alloc(class);
    VALUE  rFlags;
    VALUE  rFilename;
    int     flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    char*   filename;
    int     rc;
    am_sqlite3* am_db;


    /* at least a filename argument is required */
    rb_scan_args( argc, argv, "11", &rFilename, &rFlags );

    /* convert flags to the sqlite version */
    flags  = ( Qnil == rFlags ) ? flags : FIX2INT(rFlags);
    filename = StringValuePtr(rFilename);

    /* extract the sqlite3 wrapper struct */
    Data_Get_Struct(self, am_sqlite3, am_db);

    /* open the sqlite3 database */
    rc = sqlite3_open_v2( filename, &(am_db->db), flags, 0);
    if ( SQLITE_OK != rc ) {
        rb_raise(eAS_Error, "Failure to open database %s : [SQLITE_ERROR %d] : %s\n",
                filename, rc, sqlite3_errmsg(am_db->db));
    }

    /* by default turn on the extended result codes */
    rc = sqlite3_extended_result_codes( am_db->db, 1);
    if ( SQLITE_OK != rc ) {
        rb_raise(eAS_Error, "Failure to set extended result codes %s : [SQLITE_ERROR %d] : %s\n",
                filename, rc, sqlite3_errmsg(am_db->db));
        sqlite3_close( am_db->db );
    }

    return self;
}

/**
 * :call-seq:
 *    Amalgalite::SQLite3::Database.open16( filename ) -> SQLite3::Database
 *
 * Create a new SQLite3 database with a UTF-16 encoding
 *
 */
VALUE am_sqlite3_database_open16(VALUE class, VALUE rFilename)
{
    VALUE       self = am_sqlite3_database_alloc(class);
    char*       filename = StringValuePtr(rFilename);
    am_sqlite3* am_db;
    int         rc;

    Data_Get_Struct(self, am_sqlite3, am_db);
    rc = sqlite3_open16( filename, &(am_db->db) );
    if ( SQLITE_OK != rc ) {
        rb_raise(eAS_Error, "Failure to open UTF-16 database %s : [SQLITE_ERROR %d] : %s\n",
                filename, rc, sqlite3_errmsg( am_db->db ));
    }

    /* by default turn on the extended result codes */
    rc = sqlite3_extended_result_codes( am_db->db, 1);
    if ( SQLITE_OK != rc ) {
        rb_raise(eAS_Error, "Failure to set extended result codes on UTF-16 database %s : [SQLITE_ERROR %d] : %s\n",
                filename, rc, sqlite3_errmsg16(am_db->db));
        sqlite3_close( am_db->db );
    }

    return self;
}

/**
 * :call-seq:
 *    database.prepare( sql ) -> SQLite3::Statement
 *
 * Create a new SQLite3 statement.
 */
VALUE am_sqlite3_database_prepare(VALUE self, VALUE rSQL) 
{
    VALUE            sql = StringValue( rSQL );
    VALUE            stmt = am_sqlite3_statement_alloc(cAS_Statement); 
    am_sqlite3      *am_db;
    am_sqlite3_stmt *am_stmt;
    const char      *tail;
    int              rc;

    Data_Get_Struct(self, am_sqlite3, am_db);

    Data_Get_Struct(stmt, am_sqlite3_stmt, am_stmt);
    rc = sqlite3_prepare_v2( am_db->db, RSTRING(sql)->ptr, RSTRING(sql)->len, 
                            &(am_stmt->stmt), &tail);
    if ( SQLITE_OK != rc) {
        rb_raise(eAS_Error, "Failure to prepare statement %s : [SQLITE_ERROR %d] : %s\n",
                RSTRING(sql)->ptr, rc, sqlite3_errmsg(am_db->db));
        am_sqlite3_statement_free( am_stmt );
    }

    return stmt;
}

/***********************************************************************
 * Ruby life cycle methods
 ***********************************************************************/


/*
 * garbage collector free method for the am_data structure
 */
void am_sqlite3_database_free(am_sqlite3* wrapper)
{
    free(wrapper);
    return;
}

/*
 * allocate the am_data structure
 */
VALUE am_sqlite3_database_alloc(VALUE klass)
{
    am_sqlite3*  wrapper = ALLOC(am_sqlite3);
    VALUE   obj  = (VALUE)NULL;

    obj = Data_Wrap_Struct(klass, NULL, am_sqlite3_database_free, wrapper);
    return obj;
}