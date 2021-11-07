
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <dbus/dbus.h>

#include "eventcallbacks.h"
#include "mpris2.h"

const char *const INTERFACE_NAME = "org.mpris.MediaPlayer2";
const char *const SERVER_BUS_NAME = "org.mpris.MediaPlayer2.test";
const char *const OBJECT_PATH_NAME = "/org/mpris/MediaPlayer2/test";
const char *const METHOD_NAME = "add_numbers";

DBusError dbus_error;
void print_dbus_error (const char *str);

const char *intros = "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" \
\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\"> \
<!-- GDBus 2.68.1 --> \
<node name=\"/org/mpris/MediaPlayer2/Player\"> \
<interface name=\"org.freedesktop.DBus.Properties\"> \
<method name=\"Get\"> \
<arg type=\"s\" name=\"interface_name\" direction=\"in\"/> \
<arg type=\"s\" name=\"property_name\" direction=\"in\"/> \
<arg type=\"v\" name=\"value\" direction=\"out\"/> \
</method> \
<method name=\"GetAll\"> \
<arg type=\"s\" name=\"interface_name\" direction=\"in\"/> \
<arg type=\"a{sv}\" name=\"properties\" direction=\"out\"/> \
</method> \
<method name=\"Set\"> \
<arg type=\"s\" name=\"interface_name\" direction=\"in\"/> \
<arg type=\"s\" name=\"property_name\" direction=\"in\"/> \
<arg type=\"v\" name=\"value\" direction=\"in\"/>\
</method>\
<signal name=\"PropertiesChanged\"> \
<arg type=\"s\" name=\"interface_name\"/> \
<arg type=\"a{sv}\" name=\"changed_properties\"/>\
<arg type=\"as\" name=\"invalidated_properties\"/>\
</signal>\
</interface>\
<interface name=\"org.freedesktop.DBus.Introspectable\">\
<method name=\"Introspect\">\
<arg type=\"s\" name=\"xml_data\" direction=\"out\"/>\
</method>\
</interface>\
<interface name=\"org.freedesktop.DBus.Peer\">\
<method name=\"Ping\"/>\
<method name=\"GetMachineId\">\
<arg type=\"s\" name=\"machine_uuid\" direction=\"out\"/>\
</method>\
</interface>\
<interface name=\"org.mpris.MediaPlayer2\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
<method name=\"Raise\">\
</method>\
<method name=\"Quit\">\
</method>\
<property type=\"b\" name=\"CanQuit\" access=\"read\">\
</property>\
<property type=\"b\" name=\"CanRaise\" access=\"read\">\
</property>\
<property type=\"b\" name=\"HasTrackList\" access=\"read\">\
</property>\
<property type=\"s\" name=\"Identity\" access=\"read\">\
</property>\
<property type=\"s\" name=\"DesktopEntry\" access=\"read\">\
</property>\
<property type=\"as\" name=\"SupportedUriSchemes\" access=\"read\">\
</property>\
<property type=\"as\" name=\"SupportedMimeTypes\" access=\"read\">\
</property>\
</interface>\
<interface name=\"org.mpris.MediaPlayer2.Player\">\
<method name=\"Next\">\
</method>\
<method name=\"Previous\">\
</method>\
<method name=\"Pause\">\
</method>\
<method name=\"PlayPause\">\
</method>\
<method name=\"Stop\">\
</method>\
<method name=\"Play\">\
</method>\
<method name=\"Seek\">\
<arg type=\"x\" name=\"Offset\" direction=\"in\">\
</arg>\
</method>\
<method name=\"SetPosition\">\
<arg type=\"o\" name=\"TrackId\" direction=\"in\">\
</arg>\
<arg type=\"x\" name=\"Position\" direction=\"in\">\
</arg>\
</method>\
<method name=\"OpenUri\">\
<arg type=\"s\" name=\"Uri\" direction=\"in\">\
</arg>\
</method>\
<signal name=\"Seeked\">\
<arg type=\"x\" name=\"Position\">\
</arg>\
</signal>\
<property type=\"s\" name=\"PlaybackStatus\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"d\" name=\"Rate\" access=\"readwrite\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"a{sv}\" name=\"Metadata\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"d\" name=\"Volume\" access=\"readwrite\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"x\" name=\"Position\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"false\">\
</annotation>\
</property>\
<property type=\"d\" name=\"MinimumRate\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"d\" name=\"MaximumRate\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"b\" name=\"CanGoNext\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"b\" name=\"CanGoPrevious\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"b\" name=\"CanPlay\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"b\" name=\"CanPause\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"b\" name=\"CanSeek\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\">\
</annotation>\
</property>\
<property type=\"b\" name=\"CanControl\" access=\"read\">\
<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"false\">\
</annotation>\
</property>\
</interface>\
</node>";

DBusHandlerResult server_message_handler(DBusConnection *conn, DBusMessage *message, void *data)
{

}

const DBusObjectPathVTable server_vtable =
{
    .message_function = server_message_handler
};

event_callbacks* global_callbacks = nullptr;
void* global_userptr = nullptr;
DBusConnection *conn;

void mpris2_init(event_callbacks* callbacks, void *user)
{

    global_callbacks = callbacks;
    global_userptr = user;

    int ret;

    dbus_error_init (&dbus_error);

    conn = dbus_bus_get (DBUS_BUS_SESSION, &dbus_error);

    if (dbus_error_is_set (&dbus_error))
        print_dbus_error ("dbus_bus_get");

    if (!conn) 
        exit (1);

    // Get a well known name
    ret = dbus_bus_request_name (conn, SERVER_BUS_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &dbus_error);
    if (dbus_error_is_set (&dbus_error))
        print_dbus_error ("dbus_bus_get");


    if (!dbus_connection_register_object_path(conn, OBJECT_PATH_NAME, &server_vtable, NULL))
    {
        printf("Failed to register a object path\n");
        exit(1);
    }
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) 
    {
        fprintf (stderr, "Dbus: not primary owner, ret = %d\n", ret);
        exit (1);
    }
}
void mpris2_process_events()
{
    // Block for msg from client
    if (!dbus_connection_read_write_dispatch (conn, -1)) {
        fprintf (stderr, "Not connected now.\n");
        exit (1);
    }
 
    DBusMessage *message;

    if ((message = dbus_connection_pop_message (conn)) == NULL) {
        fprintf (stderr, "Did not get message\n");
        return;
    } 
    else
    {
        printf("nnfdfnd %s %s\n", dbus_message_get_member(message), dbus_message_get_path(message));
        const char* member = dbus_message_get_member(message);
        const char* path = dbus_message_get_path(message);
        if (strcmp(member, "Introspect") == 0 && strcmp(path,"/") == 0)
        {
            DBusMessage* reply;
            if ((reply = dbus_message_new_method_return (message)) == NULL) {
                fprintf (stderr, "Error in dbus_message_new_method_return\n");
                exit (1);
            }
            DBusMessageIter iter;
            dbus_message_iter_init_append (reply, &iter);

            if (!dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &intros)) {
                fprintf (stderr, "Error in dbus_message_iter_append_basic\n");
                exit (1);
            }

            if (!dbus_connection_send (conn, reply, NULL)) {
                fprintf (stderr, "Error in dbus_connection_send\n");
                exit (1);
            }

            dbus_connection_flush (conn);

            dbus_message_unref (reply);
        }
        else if (strcmp(member, "PlayPause") == 0)
        {
            DBusMessage* reply;
            if ((reply = dbus_message_new_method_return (message)) == NULL) {
                fprintf (stderr, "Error in dbus_message_new_method_return\n");
                exit (1);
            }
            global_callbacks->on_playpause(global_userptr);
            if (!dbus_connection_send (conn, reply, NULL)) {
                fprintf (stderr, "Error in dbus_connection_send\n");
                exit (1);
            }

            dbus_connection_flush (conn);

            dbus_message_unref (reply);

        }
        else if (strcmp(member, "Next") == 0)
        {
            DBusMessage* reply;
            if ((reply = dbus_message_new_method_return (message)) == NULL) {
                fprintf (stderr, "Error in dbus_message_new_method_return\n");
                exit (1);
            }
            global_callbacks->on_next(global_userptr);
            if (!dbus_connection_send (conn, reply, NULL)) {
                fprintf (stderr, "Error in dbus_connection_send\n");
                exit (1);
            }

            dbus_connection_flush (conn);

            dbus_message_unref (reply);

        }
        else
        {
            DBusMessage *dbus_error_msg;
            char error_msg [] = "Error in input";
            if ((dbus_error_msg = dbus_message_new_error (message, DBUS_ERROR_FAILED, error_msg)) == NULL) {
                 fprintf (stderr, "Error in dbus_mesage_new_error\n");
                 exit (1);
            }

            if (!dbus_connection_send (conn, dbus_error_msg, NULL)) {
                fprintf (stderr, "Error in dbus_connection_send\n");
                exit (1);
            }

            dbus_connection_flush (conn);
        
            dbus_message_unref (dbus_error_msg);	
        }
    }


}

void mpris2_finit()
{
}
void print_dbus_error (const char *str) 
{
    fprintf (stderr, "%s: %s\n", str, dbus_error.message);
    dbus_error_free (&dbus_error);
}
