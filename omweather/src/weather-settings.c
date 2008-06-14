/*
 * This file is part of Other Maemo Weather(omweather)
 *
 * Copyright (C) 2006 Vlad Vasiliev
 * Copyright (C) 2006 Pavel Fialko
 * 	for the code
 *        
 * Copyright (C) 2008 Andrew Zhilin
 *		      az@pocketpcrussia.com 
 *	for default icon set (Glance)
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
*/
/*******************************************************************************/
#include "weather-settings.h"
#include "weather-locations.h"
#include "weather-help.h"
#include "weather-utils.h"
#include <errno.h>
#ifdef OS2008
    #include <hildon/hildon-controlbar.h>
#else
    #include <hildon-widgets/hildon-controlbar.h>
#endif
#ifdef RELEASE
#undef DEBUGFUNCTIONCALL
#endif
#include "build"
#if defined (BSD) && !_POSIX_SOURCE
    #include <sys/dir.h>
    typedef struct dirent Dirent;
#else
    #include <dirent.h>
    #include <linux/fs.h>
    typedef struct dirent Dirent;
#endif
/*******************************************************************************/
/* Hack for Maemo SDK 2.0 */
#ifndef DT_DIR
#define DT_DIR 4
#endif
/*******************************************************************************/
void add_station_to_user_list(gchar *weather_station_name,
				gchar *weather_station_id, gboolean is_gps){
    GtkTreeIter		iter;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif    
    /* Add station to stations list */
    gtk_list_store_append(app->user_stations_list, &iter);
    gtk_list_store_set(app->user_stations_list, &iter,
#ifdef OS2008
                                0, weather_station_name,
                                1, weather_station_id,
                                2, is_gps,
#else
                                0, weather_station_name,
                                1, weather_station_id,
#endif
                                -1);
			
    /* Set it station how current (for GPS stations) */				
    if(is_gps && app->gps_must_be_current){
	if(app->config->current_station_id != NULL)
	    g_free(app->config->current_station_id);
	app->config->current_station_id = g_strdup(weather_station_id);
	if(app->config->current_station_name)
	    g_free(app->config->current_station_name);
	app->config->current_station_name = g_strdup(weather_station_name);
    }
}
/*******************************************************************************/
#ifdef OS2008
void delete_all_gps_stations(void){
    gboolean		valid;
    GtkTreeIter		iter;
    gchar		*station_name = NULL,
	    		*station_code = NULL;
    gboolean		is_gps = FALSE;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(app->user_stations_list),
                                                  &iter);
    while(valid){
    		gtk_tree_model_get(GTK_TREE_MODEL(app->user_stations_list),
                        	    &iter,
                    		    0, &station_name,
                        	    1, &station_code,
				    2, &is_gps,
                        	    -1);
    		if(is_gps){
		    if(app->config->current_station_id &&
			!strcmp(app->config->current_station_id,station_code) &&
			app->config->current_station_name && 
			!strcmp(app->config->current_station_name,station_name)){
		        /* deleting current station */
		        app->gps_must_be_current = TRUE;
		       	g_free(app->config->current_station_id);
            		g_free(app->config->current_station_name);					
		        app->config->current_station_id = NULL;
			app->config->current_station_name = NULL;
        		app->config->previos_days_to_show = app->config->days_to_show;
            	    }
		    else
			app->gps_must_be_current = FALSE;		    
		    valid = gtk_list_store_remove(app->user_stations_list, &iter);
		}else
		    valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(app->user_stations_list),
                                                        &iter);					
							
    	    }
    /* Set new current_station */
    if(!app->config->current_station_id){
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(app->user_stations_list),
                                                  &iter);
	if(valid){
	    gtk_tree_model_get(GTK_TREE_MODEL(app->user_stations_list),
                        	    &iter,
                    		    0, &station_name,
                        	    1, &station_code,
				    2, &is_gps,
                        	    -1);
	    app->config->current_station_id = g_strdup(station_code);
	    app->config->current_station_name = g_strdup(station_name);
	}		    	
    }
}
#endif
/*******************************************************************************/
void changed_country_handler(GtkWidget *widget, gpointer user_data){
    struct lists_struct	*list
			= NULL;
    GtkWidget		*config = GTK_WIDGET(user_data),
			*countries = NULL,
			*states = NULL;
    GtkTreeModel	*model;
    GtkTreeIter		iter;
    gchar		*country_name = NULL;
    long		regions_start = -1,
			regions_end = -1,
			regions_number = 0;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    list = (struct lists_struct*)g_object_get_data(G_OBJECT(config), "list");
    if(list){
	countries = list->countries;
	states = list->states;
    }
    else
	return;
    /* clear regions list */
    if(app->regions_list)
	gtk_list_store_clear(app->regions_list);
    /* clear locations list */
    if(app->stations_list)
	gtk_list_store_clear(app->stations_list);    

    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(countries), &iter)){
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(countries));
	gtk_tree_model_get(model, &iter, 0, &country_name,
					 1, &regions_start,
					 2, &regions_end,
					    -1);
	if(app->regions_list)
	    gtk_list_store_clear(app->regions_list);
	app->regions_list = create_items_list(REGIONSFILE, regions_start,
						regions_end, &regions_number);

	gtk_combo_box_set_row_span_column(GTK_COMBO_BOX(states), 0);
	gtk_combo_box_set_model(GTK_COMBO_BOX(states),
				(GtkTreeModel*)app->regions_list);

	/* if region is one then set it active and disable combobox */
	if(regions_number < 2){
	    gtk_combo_box_set_active(GTK_COMBO_BOX(states), 0);
	    gtk_widget_set_sensitive(GTK_WIDGET(states), FALSE);
	}
	else{
	    gtk_combo_box_set_active(GTK_COMBO_BOX(states), -1);
	    gtk_widget_set_sensitive(GTK_WIDGET(states), TRUE);
	}

	g_free(app->config->current_country);
	app->config->current_country = country_name;
    }
}
/*******************************************************************************/
void changed_state_handler(GtkWidget *widget, gpointer user_data){
    struct lists_struct	*list = NULL;
    GtkWidget		*config = GTK_WIDGET(user_data),
			*states = NULL,
			*stations = NULL;
    GtkTreeModel	*model = NULL;
    GtkTreeIter		iter;
    gchar		*state_name = NULL;
    long		stations_start = -1,
			stations_end = -1;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    list = (struct lists_struct*)g_object_get_data(G_OBJECT(config), "list");
    if(list){
	states = list->states;
	stations = list->stations;
    }
    else
	return;
/* clear locations list */
    if(app->stations_list)
	gtk_list_store_clear(app->stations_list);
	
    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(states), &iter)){
    	model = gtk_combo_box_get_model(GTK_COMBO_BOX(states));
	gtk_tree_model_get(model, &iter, 0, &state_name,
					 1, &stations_start,
					 2, &stations_end,
					    -1);
	/* clear locations list */
	if(app->stations_list)
	    gtk_list_store_clear(app->stations_list);

	app->stations_list = create_items_list(LOCATIONSFILE, stations_start,
						stations_end, NULL);
	gtk_combo_box_set_row_span_column(GTK_COMBO_BOX(stations), 0);
	gtk_combo_box_set_model(GTK_COMBO_BOX(stations),
				(GtkTreeModel*)app->stations_list);

	g_free(state_name);
    }
}
/*******************************************************************************/
void changed_stations_handler(GtkWidget *widget, gpointer user_data){
    struct lists_struct	*list = NULL;
    GtkWidget		*config = GTK_WIDGET(user_data),
			*stations = NULL,
			*add_button = NULL;
    GtkTreeModel	*model = NULL;
    GtkTreeIter		iter;
    gchar		*station_name = NULL,
			*station_id0 = NULL;
    double		station_latitude = 0.0F,
			station_longitude = 0.0F;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    list = (struct lists_struct*)g_object_get_data(G_OBJECT(config), "list");
    if(list)
	stations = list->stations;
    else
	return;
    add_button = lookup_widget(config, "add_from_list");
    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(stations), &iter)){
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(stations));
	gtk_tree_model_get(model, &iter, 0, &station_name,
					 1, &station_id0,
					 2, &station_latitude,
					 3, &station_longitude,
					-1);
	g_free(station_name);
	g_free(station_id0);
    }
    list->stations = stations;
    if(add_button)
	gtk_widget_set_sensitive(add_button, TRUE);
}
/*******************************************************************************/
/* Delete station from list */
void delete_station_handler(GtkButton *button, gpointer user_data){
    GtkWidget		*dialog = NULL,
			*config = GTK_WIDGET(user_data),
			*rename_entry = NULL;
    GtkTreeView		*station_list_view = NULL;
    GtkTreeIter		iter;
    gchar		*station_selected = NULL,
			*station_name = NULL,
			*station_code = NULL;
    GtkTreeModel	*model;
    GtkTreeSelection	*selection;
    gboolean		valid;
    gint		result = GTK_RESPONSE_NONE;
    GtkTreePath		*path;
#ifdef OS2008
    gboolean 		is_gps = FALSE;
#endif
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    station_list_view = (GtkTreeView*)lookup_widget(config, "station_list_view");
    rename_entry = lookup_widget(config, "rename_entry");
/* create confirm dialog */
    dialog = gtk_message_dialog_new(NULL,
                            	    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                            	    GTK_MESSAGE_QUESTION,
                            	    GTK_BUTTONS_NONE,
                            	    _("Are you sure to want delete this station ?"));
    gtk_dialog_add_button(GTK_DIALOG(dialog),
        		    _("Yes"), GTK_RESPONSE_YES);
    gtk_dialog_add_button(GTK_DIALOG(dialog),
        		    _("No"), GTK_RESPONSE_NO);
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    if(result != GTK_RESPONSE_YES)
	return;
    if(!station_list_view)
	return;
/* search station for delete */
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(station_list_view));
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(station_list_view));
    if( !gtk_tree_selection_get_selected(selection, NULL, &iter) )
	return;
 
    gtk_tree_model_get(model, &iter, 0, &station_selected, -1); 
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(app->user_stations_list),
                                                  &iter);
    while(valid){
        gtk_tree_model_get(GTK_TREE_MODEL(app->user_stations_list),
                            &iter,
#ifdef OS2008
                            0, &station_name,
                            1, &station_code,
			    2, &is_gps,
#else
                            0, &station_name,
                            1, &station_code,
#endif
                            -1);
	if(!strcmp(station_name, station_selected)){
	    path = gtk_tree_model_get_path(GTK_TREE_MODEL(app->user_stations_list),
					    &iter);
#ifdef OS2008
	    if(is_gps){
		/* Reset gps station */
		app->gps_station.id0[0] = 0;
		app->gps_station.name[0] = 0;
		app->gps_station.latitude = 0;
		app->gps_station.longtitude = 0;
	    }
#endif	    	    
	    /* delete selected station */
	    gtk_list_store_remove(app->user_stations_list, &iter);
	    g_free(station_name);
    	    g_free(station_code);
	    /* try to get previos station data */
	    if(gtk_tree_path_prev(path)){
		valid = gtk_tree_model_get_iter(GTK_TREE_MODEL(app->user_stations_list),
						&iter,
						path);
		if(valid){
		    /* set current station */
		    gtk_tree_model_get(GTK_TREE_MODEL(app->user_stations_list),
                        		&iter,
                        		0, &station_name,
                        		1, &station_code,
                        		-1);
		    /* update current station code */
        	    if(app->config->current_station_id)
            		g_free(app->config->current_station_id);
        	    app->config->current_station_id = station_code;
        	    /* update current station name */
        	    if(app->config->current_station_name)
            		g_free(app->config->current_station_name);
        	    app->config->current_station_name = station_name;
        	    app->config->previos_days_to_show = app->config->days_to_show;
		    break;
		}
		else
		    gtk_tree_path_free(path);
	    }
	    else{/* try to get next station */
		valid = gtk_tree_model_get_iter(GTK_TREE_MODEL(app->user_stations_list),
						    &iter, path);
		if(valid){
		    /* set current station */
		    gtk_tree_model_get(GTK_TREE_MODEL(app->user_stations_list),
                        		    &iter,
                        		    0, &station_name,
                        		    1, &station_code,
                        		    -1);
		    /* update current station code */
        	    if(app->config->current_station_id)
            	        g_free(app->config->current_station_id);
        	    app->config->current_station_id = station_code;
        	    /* update current station name */
        	    if(app->config->current_station_name)
            	        g_free(app->config->current_station_name);
        	    app->config->current_station_name = station_name;
        	    app->config->previos_days_to_show = app->config->days_to_show;
		    break;
		}
		else{/* if no next station than set current station to NO STATION */
		    /* update current station code */
		    gtk_tree_path_free(path);
        	    if(app->config->current_station_id)
            		g_free(app->config->current_station_id);
        	    app->config->current_station_id = NULL;
        	    /* update current station name */
        	    if(app->config->current_station_name)
            		g_free(app->config->current_station_name);
        	    app->config->current_station_name = NULL;
        	    app->config->previos_days_to_show = app->config->days_to_show;
		    /* clear rename field */
		    if(rename_entry)
			gtk_entry_set_text(GTK_ENTRY(rename_entry), "");
		    break;
		}
	    }
	}
	else{
	    g_free(station_name);
    	    g_free(station_code);
	}
	valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(app->user_stations_list),
                                                        &iter);
    }
    g_free(station_selected);
    redraw_home_window();    
    /* Update config file */
    new_config_save(app->config);
    highlight_current_station(GTK_TREE_VIEW(station_list_view));
#ifdef DEBUGFUNCTIONCALL
    END_FUNCTION;
#endif
}
/*******************************************************************************/
/*
// Update tab
// enable help for this window
    ossohelp_dialog_help_enable(GTK_DIALOG(window_config), OMWEATHER_SETTINGS_HELP_ID, app->osso);
*/
/*******************************************************************************/
/* get icon set names */
int create_icon_set_list(GtkWidget *store){
    Dirent	*dp;
    DIR		*dir_fd;
    gint	i = 0;
    char 	*temp_string = NULL;
    int		sets_number = 0;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif    
    dir_fd	= opendir(ICONS_PATH);
    if(dir_fd){
	while( (dp = readdir(dir_fd)) ){
	    if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
		continue;
	    if(dp->d_type == DT_DIR){
		gtk_combo_box_append_text(GTK_COMBO_BOX(store), dp->d_name);
		sets_number++;
		if(!strcmp(app->config->icon_set, dp->d_name))
		    gtk_combo_box_set_active(GTK_COMBO_BOX(store), i);
		i++;
	    }
	}
	closedir(dir_fd);
	/* check if selected icon set not found */
	temp_string = gtk_combo_box_get_active_text(GTK_COMBO_BOX(store));
	if(!temp_string)
	    gtk_combo_box_set_active(GTK_COMBO_BOX(store), 0);
	else 
	    g_free(temp_string);
    }
    else{
    	gtk_combo_box_append_text(GTK_COMBO_BOX(store), app->config->icon_set);
	gtk_combo_box_set_active(GTK_COMBO_BOX(store), 0);
    }
    return sets_number;
}
/*******************************************************************************/
void create_about_dialog(void){
    GtkWidget	*help_dialog,
		*notebook,
		*title;
    char	tmp_buff[2048];
    gint	result;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    help_dialog = gtk_dialog_new_with_buttons(_("Other Maemo Weather Info"),
        				NULL,
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        				_("OK"), GTK_RESPONSE_ACCEPT,
					NULL);
/* Create Notebook widget */
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(help_dialog)->vbox),
        	    notebook = gtk_notebook_new(), TRUE, TRUE, 0);
/* About tab */
    snprintf(tmp_buff, sizeof(tmp_buff) - 1,
#ifdef DISPLAY_BUILD
	    "%s%s%s%s%s%s",
#else
	    "%s%s%s%s",
#endif
	    _("\nHildon desktop applet\n"
	    "for Nokia 770/N800/N810\n"
	    "to show weather forecasts.\n"
	    "Version "), VERSION, 
#ifdef DISPLAY_BUILD
    _(" Build: "), BUILD,
#endif	    
	    _("\nCopyright(c) 2006-2008\n"
	    "Vlad Vasiliev, Pavel Fialko"),
    	    _("\nCopyright(c) 2008\n"
	    "for default icon set (Glance)\n"
	    "Andrew Zhilin")
	    );
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        			create_scrolled_window_with_text(tmp_buff,
						    GTK_JUSTIFY_CENTER),
				title = gtk_label_new(_("About")));
/* Authors tab */
    snprintf(tmp_buff, sizeof(tmp_buff) - 1, "%s",
		_("\nAuthor and maintenance:\n"
		"\tVlad Vasiliev, vlad@gas.by\n"
		"Maintenance:\n\tPavel Fialko, pavelnf@gmail.com\n"
		"Documentation:\n\tMarko Vertainen\n"
		"Design of default iconset:\n\tAndrew Zhilin"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        			create_scrolled_window_with_text(tmp_buff,
						    GTK_JUSTIFY_LEFT),
				title = gtk_label_new(_("Authors")));
/* Thanks tab */
    snprintf(tmp_buff, sizeof(tmp_buff) - 1, "%s",
	    _("\nEd Bartosh - for more feature requests,\n"
	    "\t\t\t\tsupport and criticism\n"
	    "Eugen Kaluta aka tren - for feature requests\n"
	    "\t\t\t\tand support\n"
	    "Maxim Kalinkevish aka spark for testing\n"
	    "Yuri Komyakov - for Nokia 770 device \n"
	    "Greg Thompson for support stations.txt file\n"
	    "Frank Persian - for idea of new layout\n"
	    "Brian Knight - for idea of iconset, criticism \n"
	    "\t\t\t\tand donation ;-)\n"));
    strcat(tmp_buff,	    
	    _("Andrew aka Tabster - for testing and ideas\n"
	    "Brad Jones aka kazrak - for testing\n"
	    "Alexis Iglauer - for testing\n"
	    "Eugene Roytenberg - for testing\n"
	    "Jarek Szczepanski aka Imrahil - for testing\n"
	    "Vladimir Shakhov aka Mendoza - for testing \n"
	    "Marc Dilon - for spell/stylecheck text of English\n"));
    strcat(tmp_buff,
	    _("Arkady Glazov aka Globster - for testing\n"
	      "Alexander Savchenko aka dizel - for testing\n"));
    strcat(tmp_buff,
	    _("Eric Link - for feature request and donation\n"));
              
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        			create_scrolled_window_with_text(tmp_buff,
						    GTK_JUSTIFY_LEFT),
        			title = gtk_label_new(_("Thanks")));
/* Translators tab */
    snprintf(tmp_buff, sizeof(tmp_buff) - 1, "%s",
	    _("French - Nicolas Granziano\n"
	      "Russian - Pavel Fialko, Vlad Vasiliev,\n\t    Ed Bartosh\n"
	      "Finnish - Marko Vertainen\n"
	      "German - Claudius Henrichs\n"
	      "Italian - Pavel Fialko, Alessandro Pasotti\n"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        			create_scrolled_window_with_text(tmp_buff,
						    GTK_JUSTIFY_LEFT),
        			title = gtk_label_new(_("Translators")));
/* enable help for this window */
    ossohelp_dialog_help_enable(GTK_DIALOG(help_dialog), OMWEATHER_ABOUT_HELP_ID,
								app->osso);
    gtk_widget_show_all(help_dialog);
/* start dialog window */
    result = gtk_dialog_run(GTK_DIALOG(help_dialog));
    gtk_widget_destroy(help_dialog);
}
/*******************************************************************************/
void station_list_view_select_handler(GtkTreeView *tree_view,
                                    			    gpointer user_data){
    GtkTreeIter		iter;
    gchar		*station_selected = NULL,
			*station_name = NULL,
			*station_code = NULL;
    gboolean		valid = FALSE;
    GtkTreeSelection	*selected_line = NULL;
    GtkTreeModel	*model = NULL;

#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    selected_line = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
    if( !gtk_tree_selection_get_selected(selected_line, NULL, &iter) )
        return;
    gtk_tree_model_get(model, &iter, 0, &station_selected, -1);

    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(app->user_stations_list),
                                                  &iter);
    while(valid){
        gtk_tree_model_get(GTK_TREE_MODEL(app->user_stations_list),
                            &iter,
                            0, &station_name,
                            1, &station_code,
                            -1);
        if(!strcmp(station_selected, station_name)){
        /* update current station code */
            if(app->config->current_station_id)
                g_free(app->config->current_station_id);
            app->config->current_station_id = station_code;
            /* update current station name */
            if(app->config->current_station_name)
                g_free(app->config->current_station_name);
            app->config->current_station_name = station_name;
	    /* add selected station name to the rename entry */
	    gtk_entry_set_text(GTK_ENTRY(user_data), station_name);
            break;
        }
	else{
	    g_free(station_name);
	    g_free(station_code);
	}
	valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(app->user_stations_list),
                                    	    &iter);
    }
    g_free(station_selected);
    redraw_home_window();
    new_config_save(app->config);
}
/*******************************************************************************/
void update_iterval_changed_handler(GtkComboBox *widget, gpointer user_data){
    time_t		update_time = 0;
    GtkLabel		*label;
    GtkTreeModel	*model;
    GtkTreeIter		iter;
    gchar		*temp_string,
			tmp_buff[100];
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    label = GTK_LABEL(user_data);

    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	gtk_tree_model_get(model, &iter, 0, &temp_string,
					 1, &update_time,
    					    -1);
	g_free(temp_string);
	if(app->config->update_interval != update_time){
	    app->config->update_interval = update_time;
	    remove_periodic_event();
	    add_periodic_event(time(NULL));
	}
	/* fill next update field */
	update_time = next_update();
	if(!update_time)
	    temp_string = _("Never");
	else{
	    tmp_buff[0] = 0;
	    strftime(tmp_buff, sizeof(tmp_buff) - 1, "%X %x",
	    		    localtime(&update_time));
	    temp_string = tmp_buff;
	}
	gtk_label_set_text(label, temp_string);
    }
}
/*******************************************************************************/
int get_active_item_index(GtkTreeModel *list, int time, const gchar *text,
						gboolean use_index_as_result){
    int		result = 0,
		index = 0;
    gboolean	valid = FALSE;
    GtkTreeIter	iter;
    gchar	*str_data = NULL;
    gint	int_data;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    valid = gtk_tree_model_get_iter_first((GtkTreeModel*)list, &iter);
    while(valid){
	gtk_tree_model_get(list, &iter, 
                    	    0, &str_data,
                    	    1, &int_data,
			    -1);
	if(text){ /* if parameter is string */
	    if(!strcmp((char*)text, str_data)){
		if(use_index_as_result)
		    result = index;
		else
		    result = int_data;
		break;
	    }
	}
	else{/* if parameter is int */
	    if(time == int_data){
		result = index;
		break;
	    }
	}
	g_free(str_data);
	str_data = NULL;
	index++;
	valid = gtk_tree_model_iter_next(list, &iter);
    }
    if(str_data)
	g_free(str_data);
    return result;
}
/*******************************************************************************/
void transparency_button_toggled_handler(GtkToggleButton *togglebutton,
                                        		    gpointer user_data){
    GtkWidget	*background_color = GTK_WIDGET(user_data);
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    if(gtk_toggle_button_get_active(togglebutton))
	gtk_widget_set_sensitive(background_color, FALSE);
    else
	gtk_widget_set_sensitive(background_color, TRUE);
}
/*******************************************************************************/
gboolean check_station_code(const gchar *station_code){
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    if(strlen((char*)station_code) < 5)
	return TRUE;
    return FALSE;
}
/*******************************************************************************/
void up_key_handler(GtkButton *button, gpointer list){
    GtkTreeView		*stations = (GtkTreeView*)list;
    GtkTreeIter		iter,
			prev_iter;
    GtkTreeSelection	*selected_line;
    GtkTreeModel	*model;
    GtkTreePath		*path;

    selected_line = gtk_tree_view_get_selection(GTK_TREE_VIEW(stations));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(stations));
    if( !gtk_tree_selection_get_selected(selected_line, NULL, &iter) )
        return;
    path = gtk_tree_model_get_path(model, &iter);
    if(!gtk_tree_path_prev(path)){
	gtk_tree_path_free(path);
	return;
    }
    else{
	if(gtk_tree_model_get_iter(model, &prev_iter, path))
	    gtk_list_store_move_before(GTK_LIST_STORE(model), &iter, &prev_iter);
    }
    gtk_tree_path_free(path);
}
/*******************************************************************************/
void down_key_handler(GtkButton *button, gpointer list){
    GtkTreeView		*stations = (GtkTreeView*)list;
    GtkTreeIter		iter,
			next_iter;
    GtkTreeSelection	*selected_line;
    GtkTreeModel	*model;
    GtkTreePath		*path;

    selected_line = gtk_tree_view_get_selection(GTK_TREE_VIEW(stations));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(stations));
    if( !gtk_tree_selection_get_selected(selected_line, NULL, &iter) )
        return;
    path = gtk_tree_model_get_path(model, &iter);
    gtk_tree_path_next(path);
    if(gtk_tree_model_get_iter(model, &next_iter, path))
	gtk_list_store_move_after(GTK_LIST_STORE(model), &iter, &next_iter);
    gtk_tree_path_free(path);
}
/*******************************************************************************/
void highlight_current_station(GtkTreeView *tree_view){
    GtkTreeIter		iter;
    gchar		*station_name = NULL,
			*station_code = NULL;
    gboolean		valid;
    GtkTreePath		*path;
    GtkTreeModel	*model;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(app->user_stations_list),
                                                  &iter);
    while(valid){
        gtk_tree_model_get(GTK_TREE_MODEL(app->user_stations_list),
                            &iter,
                            0, &station_name,
                            1, &station_code,
                            -1);
	if(!app->config->current_station_name){
	    app->config->current_station_name = station_name;
	    app->config->current_station_id = station_code;
	    break;
	}
	else{
    	    if(app->config->current_station_name && station_name &&                    
        	!strcmp(app->config->current_station_name, station_name)){
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
		path = gtk_tree_model_get_path(model, &iter);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree_view), path, NULL, FALSE);
		gtk_tree_path_free(path);
    		break;
	    }
	    else{
		g_free(station_name);
		g_free(station_code);
	    }
	}
	valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(app->user_stations_list),
                                    	    &iter);
    }
}
/*******************************************************************************/
void weather_window_settings(GtkWidget *widget, GdkEvent *event,
							    gpointer user_data){

    gint	day_number = (gint)user_data;/* last looking day on detail window */
    static struct lists_struct	list = { NULL, NULL, NULL };
    GSList	*temperature_group = NULL,
		*distance_group = NULL,
		*wind_group = NULL,
		*pressure_group = NULL;
    GtkWidget	*countries = NULL,
		*states = NULL,
		*stations = NULL,
		*station_list_view = NULL,
		*window_config = NULL,
		*notebook = NULL,
		*vbox = NULL,
		*buttons_box = NULL,
		*about_button = NULL,
		*apply_button = NULL,
		*close_button = NULL,
		*back_button = NULL,
		*left_right_hbox = NULL,
		*up_down_delete_buttons_vbox = NULL,
		*rename_entry = NULL,
		*station_name = NULL,
		*station_code = NULL,
		*add_station_button = NULL,
		*add_station_button1 = NULL,
		*add_station_button2 = NULL,
		*visible_items_number = NULL,
		*layout_type = NULL,
		*icon_set = NULL,
		*icon_size = NULL,
		*hide_station_name = NULL,
		*hide_arrows = NULL,
		*transparency = NULL,
		*celcius_temperature = NULL,
		*fahrenheit_temperature = NULL,
		*distance_meters = NULL,
		*distance_kilometers = NULL,
		*distance_miles = NULL,
		*distance_sea_miles = NULL,
		*wind_meters = NULL,
		*wind_kilometers = NULL,
		*wind_miles = NULL,
		*mb_pressure = NULL,
		*inch_pressure = NULL,
#ifdef OS2008
                *chk_gps = NULL,
		*sensor_page = NULL,
#endif
		*time_update_label = NULL,
		*interface_page = NULL,
		*units_page = NULL,
		*update_page = NULL,
		*font_color = NULL,
		*background_color = NULL,
		*chk_downloading_after_connection = NULL,
		*separate = NULL,
		*left_table = NULL,
		*right_table = NULL,
		*swap_temperature = NULL,
		*show_wind = NULL,
		*scrolled_window = NULL,
		*apply_rename_button = NULL,
		*up_station_button = NULL,
		*down_station_button = NULL,
		*delete_station_button = NULL,
		*update_time = NULL,
		*valid_time_list = NULL,
		*time_2switch_list = NULL;
#ifndef RELEASE
    char	tmp_buff[1024];
#endif
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
/* kill popup window :-) */
    if(app->popup_window)
        gtk_widget_destroy(app->popup_window);
/* Main window */
    window_config = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GLADE_HOOKUP_OBJECT_NO_REF(window_config, window_config, "window_config");
    g_object_set_data(G_OBJECT(window_config), "day_number", (gpointer)day_number);
    g_object_set_data(G_OBJECT(window_config), "list", (gpointer)&list);

    gtk_window_fullscreen(GTK_WINDOW(window_config));
    gtk_widget_show(window_config);
    /* create frame vbox */
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_config), vbox);
/* create tabs widget */
    notebook = gtk_notebook_new();
    GLADE_HOOKUP_OBJECT(window_config, notebook, "notebook");
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
/* add empty pages to the notebook */
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        	    left_right_hbox = gtk_hbox_new(FALSE, 0),
        	    gtk_label_new(_("Stations")));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        			interface_page = gtk_table_new(10, 4, FALSE),
        			gtk_label_new(_("Interface")));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        			 units_page = gtk_table_new(7, 3, FALSE),
        			gtk_label_new(_("Units")));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        			update_page = gtk_table_new(5, 2, FALSE),
        			gtk_label_new(_("Update")));
#ifdef OS2008
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        			sensor_page = create_sensor_page(window_config),
        			gtk_label_new(_("Sensor")));
#endif
#ifndef RELEASE
/* Events list tab */
    memset(tmp_buff, 0, sizeof(tmp_buff));
    print_list(tmp_buff, sizeof(tmp_buff) - 1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        			create_scrolled_window_with_text(tmp_buff,
						    GTK_JUSTIFY_LEFT),
        			gtk_label_new("Events"));
#endif
    gtk_widget_show(notebook);
/* Bottom buttons box */
    buttons_box = gtk_hbox_new(FALSE, 0);
    gtk_widget_set_size_request(buttons_box, -1, 60);
    /* Back buton */
    back_button = create_button_with_image(BUTTON_ICONS, "back", 40, FALSE);
    g_signal_connect(G_OBJECT(back_button), "button_press_event",
                        G_CALLBACK(back_button_handler),
			(gpointer)window_config);
    /* About buton */
    about_button = create_button_with_image(BUTTON_ICONS, "about", 40, FALSE);
    g_signal_connect(G_OBJECT(about_button), "button_press_event",
                        G_CALLBACK(about_button_handler), NULL);
    /* Apply button */
    apply_button = create_button_with_image(BUTTON_ICONS, "apply", 40, FALSE);
    g_signal_connect(G_OBJECT(apply_button), "button_press_event",
                        G_CALLBACK(apply_button_handler),
			(gpointer)window_config);
    /* Close button */
    close_button = create_button_with_image(BUTTON_ICONS, "close", 40, FALSE);
    g_signal_connect(G_OBJECT(close_button), "button_press_event",
                        G_CALLBACK(close_button_handler),
			(gpointer)window_config);
/* Pack buttons to the buttons box */
    gtk_box_pack_start(GTK_BOX(buttons_box), back_button, TRUE, TRUE, 25);
    gtk_box_pack_start(GTK_BOX(buttons_box), apply_button, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(buttons_box), about_button, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(buttons_box), close_button, TRUE, TRUE, 25);
/* Pack items to config window */
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), buttons_box, FALSE, FALSE, 0);
    gtk_widget_show(buttons_box);
/* Locations tab */
    /* left side */
    left_table = gtk_table_new(3, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(left_right_hbox), left_table,
			TRUE, TRUE, 0);
    gtk_table_attach_defaults(GTK_TABLE(left_table), 
				gtk_label_new(_("Arrange")),
				0, 1, 0, 1);
    /* Stations list */
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window),
					GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(GTK_WIDGET(scrolled_window), 220, 280);

    station_list_view = create_tree_view(app->user_stations_list);
    GLADE_HOOKUP_OBJECT(window_config, station_list_view, "station_list_view");
    gtk_container_add(GTK_CONTAINER(scrolled_window),
                	GTK_WIDGET(station_list_view));
    gtk_table_attach_defaults(GTK_TABLE(left_table), 
				scrolled_window,
                    		0, 1, 2, 3);
    
/* Up Station and Down Station and Delete Station Buttons */
    up_down_delete_buttons_vbox = gtk_vbox_new(FALSE, 5);
    gtk_table_attach_defaults(GTK_TABLE(left_table), 
				up_down_delete_buttons_vbox,
                    		1, 2, 2, 3);
/* prepare up_station_button */    
    up_station_button = create_button_with_image(NULL, "qgn_indi_arrow_up", 16, TRUE);
    gtk_widget_set_size_request(GTK_WIDGET(up_station_button), 30, -1);
    g_signal_connect(up_station_button, "clicked",
			G_CALLBACK(up_key_handler),
			(gpointer)station_list_view);
/* prepare down_station_button */    
    down_station_button = create_button_with_image(NULL, "qgn_indi_arrow_down", 16, TRUE);
    gtk_widget_set_size_request(GTK_WIDGET(down_station_button), 30, -1);
    g_signal_connect(down_station_button, "clicked",
		    	G_CALLBACK(down_key_handler),
			(gpointer)station_list_view);
/* prepare delete_station_button */    
    delete_station_button = create_button_with_image(BUTTON_ICONS, "red", 30, TRUE);
    gtk_widget_set_size_request(GTK_WIDGET(delete_station_button), 30, -1);
    g_signal_connect(delete_station_button, "clicked",
                	G_CALLBACK(delete_station_handler),
			(gpointer)window_config);
/* Pack Up, Down and Delete buttons */
    gtk_box_pack_start(GTK_BOX(up_down_delete_buttons_vbox), up_station_button,
                        TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(up_down_delete_buttons_vbox), delete_station_button,
                        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(up_down_delete_buttons_vbox), down_station_button,
                        TRUE, TRUE, 0);
/* Rename station entry */
    rename_entry = gtk_entry_new();
    GLADE_HOOKUP_OBJECT(window_config, rename_entry, "rename_entry");
    gtk_widget_set_name(rename_entry, "rename_entry");
    g_signal_connect(G_OBJECT(rename_entry), "changed",
			G_CALLBACK(entry_changed_handler),
			(gpointer)window_config);

    gtk_table_attach_defaults(GTK_TABLE(left_table), 
				rename_entry,
				0, 1, 1, 2);
/* Rename apply button */
    apply_rename_button =
	    create_button_with_image(BUTTON_ICONS, "apply", 30, FALSE);
    GLADE_HOOKUP_OBJECT(window_config, apply_rename_button , "apply_rename_button_name");
    gtk_widget_set_name(apply_rename_button, "apply_rename_button");
    g_signal_connect(G_OBJECT(apply_rename_button), "button_press_event",
                	G_CALLBACK(rename_button_handler),
			(gpointer)window_config);
    gtk_table_attach_defaults(GTK_TABLE(left_table), 
				apply_rename_button,
				1, 2, 1, 2);
    gtk_widget_set_sensitive(GTK_WIDGET(apply_rename_button), FALSE);
    /* right side */
    right_table = gtk_table_new(9, 3, FALSE);
    gtk_box_pack_start(GTK_BOX(left_right_hbox), right_table,
                        TRUE, TRUE, 0);
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				gtk_label_new(_("Add")),
        			1, 2, 0, 1);
    /* label By name */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				gtk_label_new(_("Name:")),
                    		0, 1, 1, 2);
    /* entry for station name */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				station_name = gtk_entry_new(),
                    		1, 2, 1, 2);
    GLADE_HOOKUP_OBJECT(window_config, station_name, "station_name_entry");
    gtk_widget_set_name(station_name, "station_name");
    g_signal_connect(G_OBJECT(station_name), "changed",
			G_CALLBACK(entry_changed_handler),
			(gpointer)window_config);
    /* add station button */
    add_station_button = create_button_with_image(BUTTON_ICONS, "add", 30, FALSE);
    gtk_widget_set_size_request(add_station_button, 30, 30);
    gtk_widget_set_name(add_station_button, "add_name");
    GLADE_HOOKUP_OBJECT(window_config, add_station_button, "add_station_button_name");
    gtk_widget_set_sensitive(add_station_button, FALSE);
    g_signal_connect(G_OBJECT(add_station_button), "button_press_event",
			G_CALLBACK(add_button_handler),
			(gpointer)window_config);
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				add_station_button,
				2, 3, 1, 2);
    /* label By (Zip) code */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				gtk_label_new(_("Code (Zip):")),
                    		0, 1, 2, 3);
    /* entry for station name */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				station_code = gtk_entry_new(),
                    		1, 2, 2, 3);
    gtk_widget_set_name(station_code, "station_code");
    GLADE_HOOKUP_OBJECT(window_config, station_code, "station_code_entry");
    g_signal_connect(G_OBJECT(station_code), "changed",
			G_CALLBACK(entry_changed_handler),
			(gpointer)window_config);

    /* add button */
    add_station_button1 = create_button_with_image(BUTTON_ICONS, "add", 30, FALSE);
    gtk_widget_set_size_request(add_station_button1, 30, 30);
    gtk_widget_set_name(add_station_button1, "add_code");
    GLADE_HOOKUP_OBJECT(window_config, add_station_button1, "add_code_button_name");
    gtk_widget_set_sensitive(add_station_button1, FALSE);
    g_signal_connect(G_OBJECT(add_station_button1), "button_press_event",
			G_CALLBACK(add_button_handler),
			(gpointer)window_config);
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				add_station_button1,
				2, 3, 2, 3);
#ifdef OS2008
/* GPS */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				gtk_label_new(_("Enable GPS:")),
                    		0, 1, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(right_table),
				chk_gps = gtk_check_button_new(),
				1, 2, 3, 4);
    GLADE_HOOKUP_OBJECT(window_config, chk_gps, "enable_gps");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_gps),
        			    app->config->gps_station);
#endif
    /* Label */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				gtk_label_new(_("From the list:")),
                    		1, 2, 4, 5);
    /* Countries label */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				gtk_label_new(_("Country:")),
                    		0, 1, 5, 6);
    /* countries list  */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				countries = gtk_combo_box_new_text(),
				1, 2, 5, 6);
    list.countries = countries;
    gtk_combo_box_set_row_span_column(GTK_COMBO_BOX(countries), 0);
    gtk_combo_box_set_model(GTK_COMBO_BOX(countries),
			    (GtkTreeModel*)app->countrys_list);
    gtk_widget_show(countries);
    /* States label */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				gtk_label_new(_("State:")),
                    		0, 1, 6, 7);
    /* states list */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				states = gtk_combo_box_new_text(),
				1, 2, 6, 7);
    list.states = states;
    gtk_widget_show(states);
    /* Stations label */
    gtk_table_attach_defaults(GTK_TABLE(right_table), 
				gtk_label_new(_("City:")),
                    		0, 1, 7, 8);
    /* stations list */
    gtk_table_attach_defaults(GTK_TABLE(right_table),
				stations = gtk_combo_box_new_text(),
				1, 2, 7, 8);
    list.stations = stations;
    gtk_widget_show(stations);
    GLADE_HOOKUP_OBJECT(window_config, GTK_WIDGET(stations), "stations");
    /* add button */
    add_station_button2 = create_button_with_image(BUTTON_ICONS, "add", 30, FALSE);
    gtk_widget_set_size_request(add_station_button2, 30, 30);
    gtk_widget_set_name(add_station_button2, "add_from_list");
    GLADE_HOOKUP_OBJECT(window_config, add_station_button2, "add_from_list");
    gtk_widget_set_sensitive(add_station_button2, FALSE);
    g_signal_connect(G_OBJECT(add_station_button2), "button_press_event",
			G_CALLBACK(add_button_handler),
			(gpointer)window_config);
    gtk_table_attach_defaults(GTK_TABLE(right_table),
				add_station_button2,
				2, 3, 7, 8);
    /* Set size */
    gtk_widget_set_size_request(countries, 300, -1);
    gtk_widget_set_size_request(states, 300, -1);
    gtk_widget_set_size_request(stations, 300, -1);
/* Set default value to country combo_box */
    gtk_combo_box_set_active(GTK_COMBO_BOX(countries),
				get_active_item_index((GtkTreeModel*)app->countrys_list,
				-1, app->config->current_country, TRUE));
    /* fill states list */
    changed_country_handler(NULL, (gpointer)window_config);
    /* fill stations list */
    changed_state_handler(NULL, (gpointer)window_config);
    g_signal_connect(countries, "changed",
            		G_CALLBACK(changed_country_handler),
			(gpointer)window_config);
    g_signal_connect(states, "changed",
                	G_CALLBACK(changed_state_handler),
			(gpointer)window_config);
    g_signal_connect(stations, "changed",
            		G_CALLBACK(changed_stations_handler),
			(gpointer)window_config);
    g_signal_connect(station_list_view, "cursor-changed",
                        G_CALLBACK(station_list_view_select_handler),
			rename_entry);
/* Interface tab */
    /* Visible items */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				gtk_label_new(_("Visible items:")),
				0, 1, 0, 1);
    /* Visible items number */
    visible_items_number = hildon_controlbar_new();
    GLADE_HOOKUP_OBJECT(window_config, visible_items_number, "visible_items_number");
    hildon_controlbar_set_min(HILDON_CONTROLBAR(visible_items_number), 1);
    hildon_controlbar_set_max(HILDON_CONTROLBAR(visible_items_number),
				Max_count_weather_day);
    hildon_controlbar_set_value(HILDON_CONTROLBAR(visible_items_number),
				    app->config->days_to_show);
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				visible_items_number,
				1, 2, 0, 1);
    /* Layout */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				gtk_label_new(_("Layout:")),
				0, 1, 1, 2);
    layout_type = gtk_combo_box_new_text();
    GLADE_HOOKUP_OBJECT(window_config, layout_type, "layout_type");
    gtk_combo_box_append_text(GTK_COMBO_BOX(layout_type), _("One row"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(layout_type), _("One column"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(layout_type), _("Two rows"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(layout_type), _("Two columns"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(layout_type), _("Combination"));    
    switch(app->config->icons_layout){
	default:
	case ONE_ROW: 	  gtk_combo_box_set_active(GTK_COMBO_BOX(layout_type), 0);break;
	case ONE_COLUMN:  gtk_combo_box_set_active(GTK_COMBO_BOX(layout_type), 1);break;
	case TWO_ROWS:    gtk_combo_box_set_active(GTK_COMBO_BOX(layout_type), 2);break;
	case TWO_COLUMNS: gtk_combo_box_set_active(GTK_COMBO_BOX(layout_type), 3);break;
	case COMBINATION: gtk_combo_box_set_active(GTK_COMBO_BOX(layout_type), 4);break;	
    }
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				layout_type,
				1, 2, 1, 2);
    /* Icon set */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				gtk_label_new(_("Icon set:")),
				0, 1, 2, 3);
    icon_set = gtk_combo_box_new_text();
    GLADE_HOOKUP_OBJECT(window_config, icon_set, "icon_set");
/* add icons set to list */
    if(create_icon_set_list(icon_set) < 2)
	gtk_widget_set_sensitive(icon_set, FALSE);
    else
	gtk_widget_set_sensitive(icon_set, TRUE);
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				icon_set,
				1, 2, 2, 3);
    /* Icon size */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				gtk_label_new(_("Icon size:")),
				0, 1, 3, 4);
    icon_size = hildon_controlbar_new();
    GLADE_HOOKUP_OBJECT(window_config, icon_size, "icon_size");
    hildon_controlbar_set_min(HILDON_CONTROLBAR(icon_size), 1);
    hildon_controlbar_set_max(HILDON_CONTROLBAR(icon_size), 5);
    switch(app->config->icons_size){
	case TINY: hildon_controlbar_set_value(HILDON_CONTROLBAR(icon_size),
						TINY);
	break;
	case SMALL: hildon_controlbar_set_value(HILDON_CONTROLBAR(icon_size),
						SMALL);
	break;
	case MEDIUM: hildon_controlbar_set_value(HILDON_CONTROLBAR(icon_size),
						MEDIUM);
	break;
	default:
	case LARGE: hildon_controlbar_set_value(HILDON_CONTROLBAR(icon_size),
						LARGE);
	break;
	case GIANT: hildon_controlbar_set_value(HILDON_CONTROLBAR(icon_size),
						GIANT);
	break;
    }
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				icon_size,
				1, 2, 3, 4);
    /* Swap temperature */
    gtk_table_attach_defaults(GTK_TABLE(interface_page),
        			gtk_label_new(_("Swap hi/low temperature:")),
        			0, 1, 4, 5);
    gtk_table_attach_defaults(GTK_TABLE(interface_page),
			    swap_temperature = gtk_check_button_new(),
			    1, 2, 4, 5);
    GLADE_HOOKUP_OBJECT(window_config, swap_temperature, "swap_temperature");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(swap_temperature),
        			    app->config->swap_hi_low_temperature);
    /* Show wind */
    gtk_table_attach_defaults(GTK_TABLE(interface_page),
        			gtk_label_new(_("Show wind:")),
        			2, 3, 4, 5);
    gtk_table_attach_defaults(GTK_TABLE(interface_page),
				show_wind = gtk_check_button_new(),
				3, 4, 4, 5);
    GLADE_HOOKUP_OBJECT(window_config, show_wind, "show_wind");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_wind),
        			    app->config->show_wind);
    /* Separate weather */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
			gtk_label_new(_("Current weather on first icon:")),
			0, 1, 5, 6);
    separate = gtk_check_button_new();
    GLADE_HOOKUP_OBJECT(window_config, separate, "separate");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(separate),
        			    app->config->separate);
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				separate,
				1, 2, 5, 6);
    /* Hide station name */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				gtk_label_new(_("Hide station name:")),
				0, 1, 6, 7);
    hide_station_name = gtk_check_button_new();
    GLADE_HOOKUP_OBJECT(window_config, hide_station_name, "hide_station_name");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hide_station_name),
        			    app->config->hide_station_name);
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				hide_station_name,
				1, 2, 6, 7);
    /* Hide arrows */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				gtk_label_new(_("Hide arrows:")),
				2, 3, 6, 7);
    hide_arrows = gtk_check_button_new();
    GLADE_HOOKUP_OBJECT(window_config, hide_arrows, "hide_arrows");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hide_arrows),
        			    app->config->hide_arrows);
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				hide_arrows,
				3, 4, 6, 7);
    /* Transparency */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				gtk_label_new(_("Transparency:")),
				0, 1, 7, 8);
    transparency = gtk_check_button_new();
    GLADE_HOOKUP_OBJECT(window_config, transparency, "transparency");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(transparency),
        			    app->config->transparency);
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				transparency,
				1, 2, 7, 8);
    /* Background color */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				gtk_label_new(_("Background color:")),
				0, 1, 8, 9);
    background_color = gtk_color_button_new();
    GLADE_HOOKUP_OBJECT(window_config, background_color, "background_color");
    g_signal_connect(GTK_TOGGLE_BUTTON(transparency), "toggled",
            		    G_CALLBACK(transparency_button_toggled_handler),
			    background_color);
    gtk_color_button_set_color(GTK_COLOR_BUTTON(background_color),
				&(app->config->background_color));
    if((background_color) && app->config->transparency)
        gtk_widget_set_sensitive(background_color, FALSE);	
    else
        gtk_widget_set_sensitive(background_color, TRUE);
    gtk_button_set_relief(GTK_BUTTON(background_color), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click(GTK_BUTTON(background_color), FALSE);
    gtk_table_attach(GTK_TABLE(interface_page), 
				background_color,
				1, 2, 8, 9,
				GTK_SHRINK, GTK_SHRINK,
				0, 0);
    /* Font color */
    gtk_table_attach_defaults(GTK_TABLE(interface_page), 
				gtk_label_new(_("Font color:")),
				0, 1, 9, 10);
    /* Font color button */
    font_color = gtk_color_button_new();
    GLADE_HOOKUP_OBJECT(window_config, font_color, "font_color");
    gtk_color_button_set_color(GTK_COLOR_BUTTON(font_color),
				&(app->config->font_color));
    gtk_button_set_relief(GTK_BUTTON(font_color), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click(GTK_BUTTON(font_color), FALSE);
    gtk_table_attach(GTK_TABLE(interface_page), 
				font_color,
				1, 2, 9, 10,
				GTK_SHRINK, GTK_SHRINK,
				0, 0);
/* Units tab */
    /* temperature */
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				gtk_label_new(_("Temperature units:")),
				0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				celcius_temperature
				    = gtk_radio_button_new_with_label(NULL,
									_("Celcius")),
				1, 2, 0, 1);
    GLADE_HOOKUP_OBJECT(window_config, celcius_temperature, "temperature");
    temperature_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(celcius_temperature));
    gtk_button_set_focus_on_click(GTK_BUTTON(celcius_temperature), FALSE);
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				fahrenheit_temperature
				    = gtk_radio_button_new_with_label(temperature_group,
									_("Fahrenheit")),
				2, 3, 0, 1);
    gtk_button_set_focus_on_click(GTK_BUTTON(fahrenheit_temperature), FALSE);
    if(app->config->temperature_units == CELSIUS)
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(celcius_temperature), TRUE);
    else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fahrenheit_temperature), TRUE);
    /* distance */
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				gtk_label_new(_("Distance units:")),
				0, 1, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				distance_meters
				    = gtk_radio_button_new_with_label(NULL, _("Meters")),
				1, 2, 2, 3);
    GLADE_HOOKUP_OBJECT(window_config, distance_meters, "meters");
    distance_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(distance_meters));
    gtk_button_set_focus_on_click(GTK_BUTTON(distance_meters), FALSE);

    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				distance_kilometers
				    = gtk_radio_button_new_with_label(distance_group,
									_("Kilometers")),
				2, 3, 2, 3);
    GLADE_HOOKUP_OBJECT(window_config, distance_kilometers, "kilometers");
    gtk_button_set_focus_on_click(GTK_BUTTON(distance_kilometers), FALSE);

    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				distance_miles
				    = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(distance_kilometers)),
									_("Miles")),
				1, 2, 3, 4);
    GLADE_HOOKUP_OBJECT(window_config, distance_miles, "miles");
    gtk_button_set_focus_on_click(GTK_BUTTON(distance_miles), FALSE);

    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				distance_sea_miles
				    = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(distance_miles)),
									_("Sea miles")),
				2, 3, 3, 4);
    gtk_button_set_focus_on_click(GTK_BUTTON(distance_sea_miles), FALSE);
    switch(app->config->distance_units){
	default:
	case METERS: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(distance_meters), TRUE); break;
	case KILOMETERS: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(distance_kilometers), TRUE); break;
	case MILES: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(distance_miles), TRUE); break;
	case SEA_MILES: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(distance_sea_miles), TRUE); break;
    }
    /* wind */
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				gtk_label_new(_("Wind speed units:")),
				0, 1, 4, 5);
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				wind_meters
				    = gtk_radio_button_new_with_label(NULL, _("m/s")),
				1, 2, 4, 5);
    wind_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(wind_meters));
    GLADE_HOOKUP_OBJECT(window_config, wind_meters, "wind_meters");
    gtk_button_set_focus_on_click(GTK_BUTTON(wind_meters), FALSE);

    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				wind_kilometers
				    = gtk_radio_button_new_with_label(wind_group, _("km/h")),
				2, 3, 4, 5);
    GLADE_HOOKUP_OBJECT(window_config, wind_kilometers, "wind_kilometers");
    gtk_button_set_focus_on_click(GTK_BUTTON(wind_kilometers), FALSE);

    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				wind_miles
				    = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(wind_kilometers)),
									_("mi/h")),
				1, 2, 5, 6);
    gtk_button_set_focus_on_click(GTK_BUTTON(wind_miles), FALSE);
    switch(app->config->wind_units){
	default:
	case METERS_S: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wind_meters), TRUE); break;
	case KILOMETERS_H: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wind_kilometers), TRUE); break;
	case MILES_H: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wind_miles), TRUE); break;
    }
    /* pressure */
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				gtk_label_new(_("Pressure units:")),
				0, 1, 6, 7);
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				mb_pressure
				    = gtk_radio_button_new_with_label(NULL,
									_("mb")),
				1, 2, 6, 7);
    GLADE_HOOKUP_OBJECT(window_config, mb_pressure, "mb_pressure");
    pressure_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(mb_pressure));
    gtk_button_set_focus_on_click(GTK_BUTTON(mb_pressure), FALSE);
    gtk_table_attach_defaults(GTK_TABLE(units_page), 
				inch_pressure
				    = gtk_radio_button_new_with_label(pressure_group,
									_("inHg")),
				2, 3, 6, 7);
    gtk_button_set_focus_on_click(GTK_BUTTON(inch_pressure), FALSE);
    if(app->config->pressure_units == MB)
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mb_pressure), TRUE);
    else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(inch_pressure), TRUE);
/* Update tab */
/* auto download when connect */
    gtk_table_attach_defaults(GTK_TABLE(update_page),
        			gtk_label_new(_("Automatically update data\nwhen connecting to the Internet:")),
        			0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(update_page),
				chk_downloading_after_connection = gtk_check_button_new(),
        			1, 2, 0, 1);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_downloading_after_connection),
        			    app->config->downloading_after_connecting);
    GLADE_HOOKUP_OBJECT(window_config, chk_downloading_after_connection,
			    "download_after_connection");
    g_signal_connect(GTK_TOGGLE_BUTTON(chk_downloading_after_connection), "toggled",
            		    G_CALLBACK(chk_download_button_toggled_handler),
			    NULL);
/* Switch time to the next station */
    gtk_table_attach_defaults(GTK_TABLE(update_page),
        			gtk_label_new(_("Switch to the next station after:")),
        			0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(update_page),
				time_2switch_list = gtk_combo_box_new_text(),
        			1, 2, 1, 2);
    GLADE_HOOKUP_OBJECT(window_config, time_2switch_list, "time2switch");
    gtk_combo_box_append_text(GTK_COMBO_BOX(time_2switch_list), _("Never"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(time_2switch_list), _("10 seconds"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(time_2switch_list), _("20 seconds"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(time_2switch_list), _("30 seconds"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(time_2switch_list), _("40 seconds"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(time_2switch_list), _("50 seconds"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(time_2switch_list), _("60 seconds"));

    switch((guint)(app->config->switch_time / 10)){
	default:
	case 0: gtk_combo_box_set_active(GTK_COMBO_BOX(time_2switch_list), 0); break;
	case 1: gtk_combo_box_set_active(GTK_COMBO_BOX(time_2switch_list), 1); break;
	case 2: gtk_combo_box_set_active(GTK_COMBO_BOX(time_2switch_list), 2); break;
	case 3: gtk_combo_box_set_active(GTK_COMBO_BOX(time_2switch_list), 3); break;
	case 4: gtk_combo_box_set_active(GTK_COMBO_BOX(time_2switch_list), 4); break;
	case 5: gtk_combo_box_set_active(GTK_COMBO_BOX(time_2switch_list), 5); break;
	case 6: gtk_combo_box_set_active(GTK_COMBO_BOX(time_2switch_list), 6); break;
    }
/* Vaild time */
    gtk_table_attach_defaults(GTK_TABLE(update_page),
        			gtk_label_new(_("Valid time for current weather:")),
        			0, 1, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(update_page),
				valid_time_list = gtk_combo_box_new_text(),
        			1, 2, 2, 3);
    GLADE_HOOKUP_OBJECT(window_config, valid_time_list, "valid_time");
    gtk_combo_box_append_text(GTK_COMBO_BOX(valid_time_list), _("1 hour"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(valid_time_list), _("2 hours"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(valid_time_list), _("4 hours"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(valid_time_list), _("8 hours"));
    switch((guint)(app->config->data_valid_interval / 3600)){
	case 1:  gtk_combo_box_set_active(GTK_COMBO_BOX(valid_time_list), 0); break;
	default:
	case 2:  gtk_combo_box_set_active(GTK_COMBO_BOX(valid_time_list), 1); break;
	case 4:  gtk_combo_box_set_active(GTK_COMBO_BOX(valid_time_list), 2); break;
	case 8:  gtk_combo_box_set_active(GTK_COMBO_BOX(valid_time_list), 3); break;
    }
/* Update interval */
    gtk_table_attach_defaults(GTK_TABLE(update_page),
        			gtk_label_new(_("Updating of weather data:")),
        			0, 1, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(update_page),
				update_time = gtk_combo_box_new_text(),
        			1, 2, 3, 4);
    GLADE_HOOKUP_OBJECT(window_config, update_time, "update_time");
    gtk_table_attach_defaults(GTK_TABLE(update_page),
        			gtk_label_new(_("Next update:")),
        			0, 1, 5, 6);
    gtk_table_attach_defaults(GTK_TABLE(update_page),
				time_update_label = gtk_label_new(NULL),
        			1, 2, 5, 6);
    g_signal_connect(update_time, "changed",
                	G_CALLBACK(update_iterval_changed_handler), time_update_label);
/* Fill update time box */
    gtk_combo_box_set_row_span_column(GTK_COMBO_BOX(update_time), 0);
    gtk_combo_box_set_model(GTK_COMBO_BOX(update_time),
				(GtkTreeModel*)app->time_update_list);
    gtk_combo_box_set_active(GTK_COMBO_BOX(update_time),
    get_active_item_index((GtkTreeModel*)app->time_update_list,
				    app->config->update_interval, NULL, FALSE));

/* Highlight current station */
    highlight_current_station(GTK_TREE_VIEW(station_list_view));
    gtk_entry_set_text(GTK_ENTRY(rename_entry),
			app->config->current_station_name);
/* set current page and show it for notebook */
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),
				    app->config->current_settings_page);
/*
    gtk_widget_show_all(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),
				    app->config->current_settings_page));
*/
    gtk_widget_show_all(window_config);
}
/*******************************************************************************/
void apply_button_handler(GtkWidget *button, GdkEventButton *event,
							    gpointer user_data){
    GtkWidget		*config_window = GTK_WIDGET(user_data),
		        *rename_entry = NULL,
			*visible_items_number = NULL,
			*layout_type = NULL,
			*icon_set = NULL,
			*icon_size = NULL,
			*separate = NULL,
#ifdef OS2008
			*enable_gps = NULL,
#endif
			*swap_temperature = NULL,
			*show_wind = NULL,
			*hide_station_name = NULL,
			*hide_arrows = NULL,
			*transparency = NULL,
			*background_color = NULL,
			*font_color = NULL,
			*time2switch = NULL,
			*validtime = NULL,
#ifdef OS2008
			*use_sensor = NULL,
			*display_at = NULL,
			*sensor_update_time = NULL,
#endif
			*temperature = NULL,
			*pressure = NULL,
			*meters = NULL,
			*kilometers = NULL,
			*miles = NULL,
			*wind_meters = NULL,
			*wind_kilometers = NULL;
    gboolean		valid = FALSE;
#ifndef OS2008
    gboolean		need_correct_layout_for_OS2007 = FALSE;
#endif
    GtkTreeIter		iter;
    gchar		*new_station_name = NULL,
			*station_name = NULL,
			*temp_string = NULL;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
/* check where the station name is changed */
    rename_entry = lookup_widget(config_window, "rename_entry");
    if(rename_entry){
	new_station_name = (gchar*)gtk_entry_get_text(GTK_ENTRY(rename_entry));
	if(strcmp(app->config->current_station_name, new_station_name)){
    	    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(app->user_stations_list),
                                                  &iter);
	    while(valid){
    		gtk_tree_model_get(GTK_TREE_MODEL(app->user_stations_list),
                        	    &iter, 0, &station_name, -1);
    		if(!strcmp(app->config->current_station_name, station_name)){
		    /* update current station name */
		    g_free(station_name);
		    gtk_list_store_remove(app->user_stations_list, &iter);
                    add_station_to_user_list(g_strdup(new_station_name),
					    app->config->current_station_id, FALSE);
		    if(app->config->current_station_name)
			g_free(app->config->current_station_name);
		    app->config->current_station_name = g_strdup(new_station_name);
        	    break;
		}
		else
		    g_free(station_name);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(app->user_stations_list),
                                    	    &iter);
	    }
	}
    }
/* visible items */
    visible_items_number = lookup_widget(config_window, "visible_items_number");
    if(visible_items_number){
	if(app->config->days_to_show
		!= hildon_controlbar_get_value(HILDON_CONTROLBAR(visible_items_number))){
	    /* store previos number of icons */
	    app->config->previos_days_to_show = app->config->days_to_show;
	    app->config->days_to_show
		= hildon_controlbar_get_value(HILDON_CONTROLBAR(visible_items_number));
	    (!app->config->days_to_show) && (app->config->days_to_show = 1);
#ifndef OS2008
	    need_correct_layout_for_OS2007 = TRUE;
#endif
	}
    }
/* layout type */
    layout_type = lookup_widget(config_window, "layout_type");
    if(layout_type){
	if(app->config->icons_layout
		!= gtk_combo_box_get_active((GtkComboBox*)layout_type)){
	    app->config->icons_layout
		= gtk_combo_box_get_active((GtkComboBox*)layout_type);
#ifndef OS2008
	    need_correct_layout_for_OS2007 = TRUE;
#endif
	}
    }
/* icon set */	
    icon_set = lookup_widget(config_window, "icon_set");
    if(icon_set){
	temp_string = gtk_combo_box_get_active_text(GTK_COMBO_BOX(icon_set));
	if(strcmp(app->config->icon_set, temp_string)){
	    if(app->config->icon_set)
		g_free(app->config->icon_set);
	    app->config->icon_set = g_strdup(temp_string);
	    memset(path_large_icon, 0, sizeof(path_large_icon));
	    sprintf(path_large_icon, "%s%s/", ICONS_PATH, app->config->icon_set);
	}
	g_free(temp_string);
    }
/* icon size */
    icon_size = lookup_widget(config_window, "icon_size");
    if(icon_size){
	if(app->config->icons_size
		!= hildon_controlbar_get_value(HILDON_CONTROLBAR(icon_size))){
	    app->config->icons_size
		= hildon_controlbar_get_value(HILDON_CONTROLBAR(icon_size));
	    (!app->config->icons_size) && (app->config->icons_size = 1);
#ifndef OS2008
	    need_correct_layout_for_OS2007 = TRUE;
#endif
	}
    }
/* distance units */
    meters = lookup_widget(config_window, "meters");
    kilometers = lookup_widget(config_window, "kilometers");
    miles = lookup_widget(config_window, "miles");
    if(meters && kilometers && miles){
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(meters)))
            app->config->distance_units = METERS;
	else{
	    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(kilometers)))
        	app->config->distance_units = KILOMETERS;
	    else{
	    	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(miles)))
        	    app->config->distance_units = MILES;
		else
		    app->config->distance_units = SEA_MILES;
	    }
	}
    }
/* wind units */
    wind_meters = lookup_widget(config_window, "wind_meters");
    wind_kilometers = lookup_widget(config_window, "wind_kilometers");
    if(wind_meters && wind_kilometers){
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wind_meters)))
            app->config->wind_units = METERS_S;
	else{
	    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wind_kilometers)))
        	app->config->wind_units = KILOMETERS_H;
	    else
		app->config->wind_units = MILES_H;
	}
    }
/* pressure */
    pressure = lookup_widget(config_window, "mb_pressure");
    if(pressure){
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pressure)))
            app->config->pressure_units = MB;
	else
            app->config->pressure_units = INCH;
    }
/* temperature */
    temperature = lookup_widget(config_window, "temperature");
    if(temperature){
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(temperature)))
            app->config->temperature_units = CELSIUS;
	else
            app->config->temperature_units = FAHRENHEIT;
    }
#ifdef OS2008
/* enable gps */
    enable_gps = lookup_widget(config_window, "enable_gps");
    if(enable_gps){
	app->config->gps_station = 
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(enable_gps));
	if(app->config->gps_station)
            add_gps_event(1);
	else{/* Reset gps station */
	    app->gps_station.id0[0] = 0;
	    app->gps_station.name[0] = 0;
	    app->gps_station.latitude = 0;
	    app->gps_station.longtitude = 0;		
	}
    }
#endif
/* swap temperature */
    swap_temperature = lookup_widget(config_window, "swap_temperature");
    if(swap_temperature)
	app->config->swap_hi_low_temperature = 
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(swap_temperature));
/* show wind */
    show_wind = lookup_widget(config_window, "show_wind");
    if(show_wind)
	app->config->show_wind = 
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_wind));
/* separate */
    separate = lookup_widget(config_window, "separate");
    if(separate)
	app->config->separate = 
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(separate));
/* hide station name */
    hide_station_name = lookup_widget(config_window, "hide_station_name");
    if(hide_station_name){
	if(app->config->hide_station_name
		!= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hide_station_name))){
	    app->config->hide_station_name
		= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hide_station_name));
#ifndef OS2008
	    need_correct_layout_for_OS2007 = TRUE;
#endif
	}
    }
#ifdef OS2008
/* use sensor */
    use_sensor = lookup_widget(config_window, "use_sensor");
    if(use_sensor)
	app->config->use_sensor = 
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_sensor));
/* display sensor at */
    display_at = lookup_widget(config_window, "display_at");
    if(display_at){
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(display_at)))
	    app->config->display_at = STATION_NAME;
	else
	    app->config->display_at = ICON;
    }
/* sensor update time */
    sensor_update_time = lookup_widget(config_window, "update_time_entry");
    if(sensor_update_time && !check_entry_text(GTK_ENTRY(sensor_update_time))){
	app->config->sensor_update_time
	    = (guint)atoi(gtk_entry_get_text(GTK_ENTRY(sensor_update_time)));
	if(app->config->use_sensor){
	    g_source_remove(app->sensor_timer);
	    app->sensor_timer = g_timeout_add(app->config->sensor_update_time * 1000,
                                            (GtkFunction)read_sensor,
                                            NULL);
	}
    }
#endif
/* hide arrows */
    hide_arrows = lookup_widget(config_window, "hide_arrows");
    if(hide_arrows){
	if(app->config->hide_arrows
		!= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hide_arrows))){
	    app->config->hide_arrows
		= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hide_arrows));
#ifndef OS2008
	    need_correct_layout_for_OS2007 = TRUE;
#endif
	}
    }
/* transparency */
    transparency = lookup_widget(config_window, "transparency");
    if(transparency)
	app->config->transparency = 
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(transparency));
/* background color */
    background_color = lookup_widget(config_window, "background_color");
    if(background_color)
    	gtk_color_button_get_color(GTK_COLOR_BUTTON(background_color),
					&(app->config->background_color));
/* font color */
    font_color = lookup_widget(config_window, "font_color");
    if(font_color)
    	gtk_color_button_get_color(GTK_COLOR_BUTTON(font_color),
					&(app->config->font_color));
/* Switch time */
    time2switch = lookup_widget(config_window, "time2switch");
    if(time2switch){
	if( gtk_combo_box_get_active((GtkComboBox*)time2switch) != app->config->switch_time / 10){
	    app->config->switch_time = 10 * gtk_combo_box_get_active((GtkComboBox*)time2switch);
		g_source_remove(app->switch_timer);
		if(app->config->switch_time > 0)
		    app->switch_timer = g_timeout_add(app->config->switch_time * 1000,
                    					(GtkFunction)switch_timer_handler,
                        				app->main_window);
	}
    }
/* Data valid time */
    validtime = lookup_widget(config_window, "valid_time");
    if(validtime){
	if( (1 << gtk_combo_box_get_active((GtkComboBox*)validtime)) != app->config->data_valid_interval / 3600 ){
	    app->config->data_valid_interval = 3600 * (1 << gtk_combo_box_get_active((GtkComboBox*)validtime));
	}
    }
#ifndef OS2008
/* add param for close button handler */
    if(need_correct_layout_for_OS2007)
	g_object_set_data(G_OBJECT(config_window),
			    "need_correct_layout_for_OS2007",
			    (gpointer)1 );
#endif
/* save settings */
    new_config_save(app->config);
    redraw_home_window();
}
/*******************************************************************************/
void close_button_handler(GtkWidget *button, GdkEventButton *event,
							    gpointer user_data){
    GtkWidget	*config_window = GTK_WIDGET(user_data),
		*notebook = NULL;
    guint	current_page = 0;
    gboolean	need_update_weather = FALSE;
#ifndef OS2008
    gboolean	need_correct_layout_for_OS2007 = FALSE;
#endif
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    notebook = lookup_widget(config_window, "notebook");
    if(notebook)
	current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    
    if(g_object_get_data(G_OBJECT(user_data), "need_update_weather"))
	need_update_weather = TRUE;
#ifndef OS2008
    if(g_object_get_data(G_OBJECT(user_data), "need_correct_layout_for_OS2007"))
	need_correct_layout_for_OS2007 = TRUE;
#endif
    gtk_widget_destroy(config_window);
/* check if update is needed */
    if(need_update_weather){
	update_weather(TRUE);
	redraw_home_window();
    }
#ifndef OS2008
/* check if correct layout needed */
    if(need_correct_layout_for_OS2007)
        hildon_banner_show_information(app->main_window,
                                        NULL,
                                        _("Use Edit layout \nfor tuning images of applet"));
#endif
    app->config->current_settings_page = current_page;
/* save config */
    new_config_save(app->config);
}
/*******************************************************************************/
void about_button_handler(GtkWidget *button, GdkEventButton *event,
							    gpointer user_data){
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    create_about_dialog();
}
/*******************************************************************************/
void back_button_handler(GtkWidget *button, GdkEventButton *event,
							    gpointer user_data){
    gint day_number
	    = (gint)g_object_get_data(G_OBJECT(user_data), "day_number");
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    gtk_widget_destroy(GTK_WIDGET(user_data));
    weather_window_popup(NULL, NULL, (gpointer)day_number);
}
/*******************************************************************************/
void entry_changed_handler(GtkWidget *entry, gpointer user_data){
    gchar	*changed_entry_name = NULL;
    GtkWidget	*button = NULL,
		*config_window = GTK_WIDGET(user_data);
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    /* get pressed gtkedit name */    
    changed_entry_name = (gchar*)gtk_widget_get_name(GTK_WIDGET(entry));

    if(!changed_entry_name)
        return;
        
    if( !strcmp(changed_entry_name, "rename_entry") ){/* check rename entry */
	button = lookup_widget(config_window, "apply_rename_button_name");
	if(button){
	    if(strcmp((char*)gtk_entry_get_text(GTK_ENTRY(entry)),
			app->config->current_station_name))
		gtk_widget_set_sensitive(button, TRUE);
	    else
		gtk_widget_set_sensitive(button, FALSE);
	}
    }
    else{
	if( !strcmp(changed_entry_name, "station_code") )/* check code entry */
	    button = lookup_widget(config_window, "add_code_button_name");
	else
	    if( !strcmp(changed_entry_name, "station_name") )/* check name entry */
		button = lookup_widget(config_window, "add_station_button_name");
	/* Change sensitive of button */
	if(strlen(gtk_entry_get_text(GTK_ENTRY(entry))) > 0)
	    gtk_widget_set_sensitive(button, TRUE);
	else
	    gtk_widget_set_sensitive(button, FALSE);
    }
}
/*******************************************************************************/
int lookup_and_select_station(gchar *station_name, Station *result){

    FILE		*fh_region, *fh_station;
    Region_item  	region;
    GtkListStore	*list = NULL;
    GtkTreeIter		iter;
    char		buffer[512];
    char		buffer_full_name[2048];
    Station		station;
    GtkWidget		*window_select_station = NULL,
			*station_list_view = NULL,
			*scrolled_window = NULL,
			*label = NULL,
			*table = NULL;
    gchar		*selected_station_name = NULL;
    long		max_bytes = 0,
			readed_bytes = 0;
    gboolean		valid;
    GtkTreeModel	*model;
    GtkTreeSelection	*selection;
    gchar        	*station_full_name = NULL,
			*station_name_temp = NULL,
                	*station_id0 = NULL;
    double       	station_latitude,
        		station_longtitude;
    int 		return_code = 0;

    /* Prepare */
    memset(result->name, 0, sizeof(result->name));
    memset(result->id0, 0, sizeof(result->id0));

    fh_region = fopen(REGIONSFILE, "rt");
    if(!fh_region){
	fprintf(stderr, "\nCan't read file %s: %s", REGIONSFILE,
		strerror(errno));
	return -1;	
    }
    list = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING,
				 G_TYPE_DOUBLE, G_TYPE_DOUBLE,
				 G_TYPE_STRING);
    /* Reading region settings */
    while(!feof(fh_region)){
	memset(buffer, 0, sizeof(buffer));
	fgets(buffer, sizeof(buffer) - 1, fh_region);
        parse_region_string(buffer,&region);
	fh_station = fopen(LOCATIONSFILE, "rt");
	if(!fh_station){
	    fprintf(stderr, "\nCan't read file LOCATIONSFILE: %s", 
		    strerror(errno));
	    return -1;	
	}
	max_bytes = region.end - region.start;
	readed_bytes = 0;
	if(region.start > -1)
	    if(fseek(fh_station, region.start, SEEK_SET)){
		fprintf(stderr,
			"\nCan't seek to the position %ld on LOCATIONSFILE file: %s\n",
			region.start, strerror(errno));
		return -1;
	    }
	
	while(!feof(fh_station)){
	    memset(buffer, 0, sizeof(buffer));
	    fgets(buffer, sizeof(buffer) - 1, fh_station);
	    readed_bytes += strlen(buffer);
	    if(!parse_station_string(buffer, &station)){
		if (strcasestr(station.name,station_name)){
		    gtk_list_store_append(list, &iter);
		    snprintf(buffer_full_name,sizeof(buffer_full_name) - 1,
					    "%s,%s",region.name,station.name);
		    gtk_list_store_set(list, &iter,
					0, buffer_full_name,
					1, station.id0,
					2, station.latitude,
					3, station.longtitude,
					4, station.name,
					-1);
		}
	    }
	    if(region.start > -1 && region.end > -1 && readed_bytes >= max_bytes)
		break;
	}
	fclose(fh_station);
    }
    fclose(fh_region);
    /* Create dialog window */
    window_select_station = gtk_dialog_new_with_buttons(_("Select Station"),
        						NULL,
							GTK_DIALOG_MODAL,
							NULL);
    /* Add buttons */
    gtk_dialog_add_button(GTK_DIALOG(window_select_station),
        		    _("OK"), GTK_RESPONSE_ACCEPT);
    gtk_dialog_add_button(GTK_DIALOG(window_select_station),
        		    _("Cancel"), GTK_RESPONSE_REJECT);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window_select_station)->vbox),
        		    table = gtk_table_new(2, 2, FALSE), TRUE, TRUE, 0);	    

   /* Add Label and Edit field for station name */
    gtk_table_attach_defaults(GTK_TABLE(table),
        			label = gtk_label_new(_("List of the found station(s):")),
        			0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table),
        			label = gtk_alignment_new(0.f, 0.f, 0.f, 0.f),
        			0, 1, 1, 2);
    /* Stations list */
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window),
					GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(GTK_WIDGET(scrolled_window), 500, 280);

    station_list_view = create_tree_view(list);
    gtk_container_add(GTK_CONTAINER(scrolled_window),
                	GTK_WIDGET(station_list_view));

    gtk_container_add(GTK_CONTAINER(label), scrolled_window);
    /* set size for dialog */
    gtk_widget_set_size_request(GTK_WIDGET(window_select_station), 550, -1);
    gtk_widget_show_all(window_select_station);

    /* start dialog */
    switch(gtk_dialog_run(GTK_DIALOG(window_select_station))){
	case GTK_RESPONSE_ACCEPT:/* Press Button Ok */
	    /* Lookup selected item */
	    model = gtk_tree_view_get_model(GTK_TREE_VIEW(station_list_view));
	    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(station_list_view));
	    if( !gtk_tree_selection_get_selected(selection, NULL, &iter) )
		return -1;
	    gtk_tree_model_get(model, &iter, 0, &selected_station_name, -1); 
	    
	    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list),
                                                  &iter);
	    while(valid){
    		gtk_tree_model_get(GTK_TREE_MODEL(list),
                        	    &iter,
                        	    0, &station_full_name,
				    1, &station_id0,
				    2, &station_latitude,
				    3, &station_longtitude,
				    4, &station_name_temp,
                        	    -1);
    		if(!strcmp(selected_station_name, station_full_name)){
    		    /* copy selected station to result */
    		    memcpy(result->name, station_name_temp, ((sizeof(result->name) - 1) > ((int)strlen(station_name_temp)) ?
				    ((int)strlen(station_name_temp)) : (sizeof(result->name) - 1)));
    		    memcpy(result->id0, station_id0, ((sizeof(result->id0) - 1) > ((int)strlen(station_id0)) ?
				    ((int)strlen(station_id0)) : (sizeof(result->id0) - 1)));
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(list),
                                                        &iter);
    	    }
    	    return_code = 0;
	break;
	default: 
	    return_code = -1;
	break;
    }
    if(selected_station_name)
        g_free(selected_station_name);
    gtk_widget_destroy(window_select_station);

    return return_code;
}
/*******************************************************************************/
void add_button_handler(GtkWidget *button, GdkEventButton *event,
							    gpointer user_data){
    GtkWidget		*config = GTK_WIDGET(user_data),
			*station_name_entry = NULL,
		        *station_code_entry = NULL,
			*stations = NULL,
			*stations_list_view = NULL;
    gchar		*pressed_button = NULL;
    GtkTreeModel	*model = NULL;
    GtkTreeIter		iter;
    gchar		*station_name = NULL,
			*station_code = NULL;
    gboolean            station_code_invalid = TRUE;
    Station		select_station;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    /* get pressed button name */    
    pressed_button = (gchar*)gtk_widget_get_name(GTK_WIDGET(button));

    if (!pressed_button)
        return;

    if  ( !strcmp((char*)pressed_button, "add_name")){
	station_name_entry = lookup_widget(config, "station_name_entry");
	if (lookup_and_select_station((gchar*)gtk_entry_get_text((GtkEntry*)station_name_entry),&select_station)==0){
	        add_station_to_user_list(g_strdup(select_station.name),
	                                 g_strdup(select_station.id0),
	                                 FALSE);
	        new_config_save(app->config);
	
	    gtk_entry_set_text(((GtkEntry*)station_name_entry),"");
	}
	  
    }
    else{
        if (!strcmp((char*)pressed_button, "add_code")){
	    station_code_entry = lookup_widget(config, "station_code_entry");
	    station_code_invalid = check_station_code(gtk_entry_get_text((GtkEntry*)station_code_entry));
	    if(!station_code_invalid){
	        add_station_to_user_list(g_strdup(gtk_entry_get_text((GtkEntry*)station_code_entry)),
	                                 g_strdup(gtk_entry_get_text((GtkEntry*)station_code_entry)), 
	                                 FALSE);
	        new_config_save(app->config);
	        gtk_entry_set_text(((GtkEntry*)station_code_entry),"");
	        /* disable add button */
		gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
	    }else{
	    /* Need Error window */
	    }
        }
        else{
	    stations = lookup_widget(config, "stations");
	    if(stations){
	        if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(stations), &iter)){
		    model = gtk_combo_box_get_model(GTK_COMBO_BOX(stations));
		    gtk_tree_model_get(model, &iter, 0, &station_name,
					   1, &station_code, -1);
		    add_station_to_user_list(station_name, station_code, FALSE);
		    g_free(station_name);
		    g_free(station_code);
		    new_config_save(app->config);
		    /* set selected station to nothing */
		    gtk_combo_box_set_active((GtkComboBox*)stations, -1);
		    /* disable add button */
		    gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
		}
	    }
	}
    }
/* set need update weather flag for close button handler */
    g_object_set_data(G_OBJECT(config),
			"need_update_weather",
			(gpointer) 1);
    stations_list_view = lookup_widget(config, "station_list_view");
    if(stations_list_view){
	highlight_current_station(GTK_TREE_VIEW(stations_list_view));
	redraw_home_window();
    }
}
/*******************************************************************************/
void rename_button_handler(GtkWidget *button, GdkEventButton *event,
							    gpointer user_data){
    GtkWidget		*config_window = GTK_WIDGET(user_data),
		        *rename_entry = NULL;
    gboolean		valid = FALSE;
    GtkTreeIter		iter;
    gchar		*new_station_name = NULL,
			*station_name = NULL;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
/* check where the station name is changed */
    rename_entry = lookup_widget(config_window, "rename_entry");
    if(rename_entry){
	new_station_name = (gchar*)gtk_entry_get_text(GTK_ENTRY(rename_entry));
	if(strcmp(app->config->current_station_name, new_station_name)){
    	    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(app->user_stations_list),
                                                  &iter);
	    while(valid){
    		gtk_tree_model_get(GTK_TREE_MODEL(app->user_stations_list),
                        	    &iter, 0, &station_name, -1);
    		if(!strcmp(app->config->current_station_name, station_name)){
		    /* update current station name */
		    g_free(station_name);
		    gtk_list_store_remove(app->user_stations_list, &iter);
                    add_station_to_user_list(g_strdup(new_station_name),
					    app->config->current_station_id, FALSE);
		    if(app->config->current_station_name)
			g_free(app->config->current_station_name);
		    app->config->current_station_name = g_strdup(new_station_name);
		    gtk_widget_set_sensitive(button, FALSE);
        	    break;
		}
		else
		    g_free(station_name);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(app->user_stations_list),
                                    	    &iter);
	    }
	}
    }
/* save settings */
    new_config_save(app->config);
    redraw_home_window();
}
/*******************************************************************************/
void chk_download_button_toggled_handler(GtkRadioButton *button,
							    gpointer user_data){
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) ){
	app->config->downloading_after_connecting = TRUE;
    }
    else
	app->config->downloading_after_connecting = FALSE;
}
/*******************************************************************************/
