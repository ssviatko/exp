#include <string.h>
#include <gtk/gtk.h>

// global declarations for login window
GtkWidget *window_login;
GtkWidget *check_default; // checkbox for make default
GtkWidget *check_rememberpw; // checkbox for remember password

// Login window support functions
static void button_login_clicked(GtkWidget *widget,gpointer data)
{
	g_print("You clicked Login!\n");\
	gtk_widget_destroy(window_login);
}

static void button_quit_clicked(GtkWidget *widget,gpointer data)
{
	g_print("You clicked Quit!\n");
	gtk_widget_destroy(window_login);
}

static gboolean window_login_delete_event(GtkWidget *widget,GdkEvent *event,gpointer data)
{
	g_print("delete event occurred\n");
	return FALSE; // change to TRUE to intercept quit function of closebox
}

static void window_login_destroy(GtkWidget *widget,gpointer data)
{
	// the destroy event occurs when we call gtk_widget_destroy() on the window,
	// or if we return FALSE in the delete_event handler.
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_default)))
		g_print("saving defaults\n");
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_rememberpw)))
		g_print("remembering password\n");

	gtk_main_quit();
}

int main(int argc, char **argv)
{
	// declarations for login window
	GtkWidget *button_login,*button_quit; // login window action buttons
	GtkWidget *box_login; // login window vbox
	GtkWidget *label_login; // Please login: message
	GtkWidget *label_host,*label_db,*label_user,*label_pw;
	GtkWidget *entry_host,*entry_db,*entry_user,*entry_pw;
	GtkWidget *label_cred;
	GtkWidget *table_login;

	gtk_init(&argc,&argv);

	// create login window and login master box
	window_login=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window_login),"Database Login");
	gtk_window_set_resizable(GTK_WINDOW(window_login),FALSE);
	g_signal_connect(G_OBJECT(window_login),"delete_event",G_CALLBACK(window_login_delete_event),NULL);
	g_signal_connect(G_OBJECT(window_login),"destroy",G_CALLBACK(window_login_destroy),NULL);
	gtk_container_set_border_width(GTK_CONTAINER(window_login),1);
	box_login=gtk_vbox_new(FALSE,5); // hetero spacing, 5 padding
	gtk_widget_show(box_login);

	// create login "action table" with buttons/text entries
	// table
	table_login=gtk_table_new(8,3,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table_login),5);
	gtk_table_set_col_spacings(GTK_TABLE(table_login),5);
	gtk_widget_show(table_login);
	// login label
	label_login=gtk_label_new("Please login to the database");
	gtk_table_attach_defaults(GTK_TABLE(table_login),label_login,0,3,0,1);
	gtk_widget_show(label_login);
	// host
	label_host=gtk_label_new("Host:");
	gtk_misc_set_alignment(GTK_MISC(label_host),1,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table_login),label_host,0,1,1,2);
	entry_host=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_host),80);
	gtk_entry_set_text(GTK_ENTRY(entry_host),"localhost");
	gtk_table_attach_defaults(GTK_TABLE(table_login),entry_host,1,3,1,2);
	gtk_widget_show(label_host);
	gtk_widget_show(entry_host);
	// db
	label_db=gtk_label_new("Database:");
	gtk_misc_set_alignment(GTK_MISC(label_db),1,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table_login),label_db,0,1,2,3);
	entry_db=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_db),80);
	gtk_entry_set_text(GTK_ENTRY(entry_db),"atelier");
	gtk_table_attach_defaults(GTK_TABLE(table_login),entry_db,1,3,2,3);
	gtk_widget_show(label_db);
	gtk_widget_show(entry_db);
	// username
	label_user=gtk_label_new("Username:");
	gtk_misc_set_alignment(GTK_MISC(label_user),1,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table_login),label_user,0,1,3,4);
	entry_user=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_user),80);
	gtk_entry_set_text(GTK_ENTRY(entry_user),"ssviatko");
	gtk_table_attach_defaults(GTK_TABLE(table_login),entry_user,1,3,3,4);
	gtk_widget_show(label_user);
	gtk_widget_show(entry_user);
	// password
	label_pw=gtk_label_new("Password:");
	gtk_misc_set_alignment(GTK_MISC(label_pw),1,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table_login),label_pw,0,1,4,5);
	entry_pw=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_pw),80);
	gtk_entry_set_visibility(GTK_ENTRY(entry_pw),FALSE); // make a password char appear when typed
	gtk_entry_set_invisible_char(GTK_ENTRY(entry_pw),'*'); // set that char to a star
	gtk_table_attach_defaults(GTK_TABLE(table_login),entry_pw,1,3,4,5);
	gtk_widget_show(label_pw);
	gtk_widget_show(entry_pw);
	// default settings checkbox
	label_cred=gtk_label_new("Credentials:");
	gtk_misc_set_alignment(GTK_MISC(label_cred),1,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table_login),label_cred,0,1,5,6);
	check_default=gtk_check_button_new_with_mnemonic("Make _default");
	gtk_table_attach_defaults(GTK_TABLE(table_login),check_default,1,3,5,6);
	gtk_widget_show(label_cred);
	gtk_widget_show(check_default);
	// remember password checkbox
	check_rememberpw=gtk_check_button_new_with_mnemonic("Remember _password");
	gtk_table_attach_defaults(GTK_TABLE(table_login),check_rememberpw,1,3,6,7);
	gtk_widget_show(check_rememberpw);
	// quit button
	button_quit=gtk_button_new_with_label("Quit");
	g_signal_connect(G_OBJECT(button_quit),"clicked",G_CALLBACK(button_quit_clicked),NULL);
	gtk_table_attach_defaults(GTK_TABLE(table_login),button_quit,0,1,7,8);
	gtk_widget_show(button_quit);
	// login button
	button_login=gtk_button_new_with_label("Login");
	g_signal_connect(G_OBJECT(button_login),"clicked",G_CALLBACK(button_login_clicked),NULL);
	gtk_table_attach_defaults(GTK_TABLE(table_login),button_login,2,3,7,8);
	gtk_widget_show(button_login);

	gtk_box_pack_start(GTK_BOX(box_login),table_login,TRUE,TRUE,0);
	gtk_container_add(GTK_CONTAINER(window_login),box_login);
	gtk_window_set_position(GTK_WINDOW(window_login),GTK_WIN_POS_CENTER_ALWAYS); // screen center position
	gtk_widget_grab_focus(GTK_WIDGET(entry_user)); // focus the username field
	gtk_widget_show(window_login);

	gtk_main();
	g_print("If logged in properly, progressing to app...\n");
	
	return 0;
}
