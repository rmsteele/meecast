/*
 * This file is part of Other Maemo Weather(omweather)
 *
 * Copyright (C) 2006 Vlad Vasiliev
 * Copyright (C) 2006 Pavel Fialko
 * 	for the code
 *        
 * Copyright (C) Superkaramba's Liquid Weather ++ team
 *	for ALL the artwork (icons)
 *        The maintainer (also the main author I believe)
 *        is Matthew <dmbkiwi@yahoo.com>
 *  http://liquidweather.net/icons.html#iconsets
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
#include "weather-window_popup.h"
/*******************************************************************************/
static gboolean popup_window_event_cb(GtkWidget *widget, 
                    			GdkEvent *event, 
                    			gpointer user_data){
    gint x, y, w, h;
    gtk_widget_get_pointer(widget, &x, &y);
    w = widget->allocation.width;
    h = widget->allocation.height;
    if(!( (x >= 0) && (x <= w) && (y >= 0) && (y <= h) )){
        gtk_widget_destroy(app->popup_window);
	app->popup_window=NULL;
    }	
    return TRUE; 
}
/*******************************************************************************/
void pre_update_weather(void){
    if (!app->dbus_is_initialize)
	weather_initialize_dbus();
    app->show_update_window = TRUE;
    update_weather();
}
/*******************************************************************************/
/* Show extended information about weather */
void weather_window_popup_show(GtkWidget *widget,
                    		GdkEvent *event,
                    		gpointer user_data){

    int		i;
    time_t	current_time = 0,
		utc_time;
    GtkWidget	*popup_frame,
		*popup_vbox,
		*separator_after_header,
		*separator_after_sun_time,
		*separator_after_current,
		*separator_after_temperature,
		*separator_after_24_hours_widget;
    gboolean	first_day = FALSE,
		second_day = FALSE;
    
    /* if no one station present in list show only preference */
    if(!app->config->current_station_id){
	weather_window_settings(widget, event, user_data);
	return;
    }
    /* Search: Which button is pressed */
    for(i = 0;i < app->config->days_to_show; i++)
	if(app->buttons[i]->button == widget) 
	    break;
    /* Not found pressed button */
    if( i >= app->config->days_to_show )
	return;

/* Create POPUP WINDOW */ 
    app->popup_window = gtk_window_new( GTK_WINDOW_POPUP );
    gtk_window_set_decorated(GTK_WINDOW(app->popup_window), FALSE);
/* create popup window frame */
    popup_frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(app->popup_window), popup_frame);
/* create frame vbox */    
    popup_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(popup_frame), popup_vbox);
    /* prepare time additioanal value */
    current_time = time(NULL); /* get current day */
    /* correct time for current location */
    utc_time = mktime(gmtime(&current_time));
    current_time = utc_time + app->weather_days[boxs_offset[i]].zone;

/* check if fist day is pressed */
    if(i == 0){
	first_day = TRUE;
	gtk_window_move(GTK_WINDOW(app->popup_window), 180, 60);
    }
    else
	gtk_window_move(GTK_WINDOW(app->popup_window), 180, 140);
/* check if second day is pressed */
    if(i == 1)
	second_day = TRUE;
    i = boxs_offset[i];
/* added header to popup window */
    gtk_box_pack_start(GTK_BOX(popup_vbox), create_header_widget(i),
			    FALSE, FALSE, 0);
/* added separator */
    separator_after_header = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_header,
			    FALSE, FALSE, 0);
    if((i < Max_count_weather_day) && strcmp(app->weather_days[i].dayfuname, "")){
	gtk_box_pack_start(GTK_BOX(popup_vbox), create_sun_time_widget(i),
					FALSE, FALSE, 0);
	separator_after_sun_time = gtk_hseparator_new();                                                                    
        gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_sun_time,
                                            FALSE, FALSE, 0);
	if(first_day || (second_day && app->config->separate)){ /* if first or second day */
	    if(first_day){
		if(!app->config->separate){ /* if weather data isn't separated */
		    gtk_box_pack_start(GTK_BOX(popup_vbox), create_temperature_range_widget(i),
    					    FALSE, FALSE, 0);
		    separator_after_temperature = gtk_hseparator_new();
		    gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_temperature,
					    FALSE, FALSE, 0);
		    if((app->weather_current_day.date_time  > ( current_time - app->config->data_valid_interval - 3600)) &&
        		    (app->weather_current_day.date_time  < ( current_time + app->config->data_valid_interval + 3600)) &&
			    app->weather_current_day.location){
			gtk_box_pack_start(GTK_BOX(popup_vbox), create_current_weather_widget(),
						    FALSE, FALSE, 0);
			/* added separator */
			separator_after_current = gtk_hseparator_new();
			gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_current,
			    			FALSE, FALSE, 0);
		    }
		    gtk_box_pack_start(GTK_BOX(popup_vbox), create_24_hours_widget(i, time(NULL)), /* time(NULL) - current local time */
					    FALSE, FALSE, 0);
		    separator_after_24_hours_widget = gtk_hseparator_new();
		    gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_24_hours_widget,
					FALSE, FALSE, 0);		
		}
		else{/* if weather data is separated */
		    if((app->weather_current_day.date_time > ( current_time - app->config->data_valid_interval - 3600)) &&
        		    (app->weather_current_day.date_time < ( current_time + app->config->data_valid_interval + 3600)) &&
			    app->weather_current_day.location){
			gtk_box_pack_start(GTK_BOX(popup_vbox), create_current_weather_widget(),
						    FALSE, FALSE, 0);
			/* added separator */
			separator_after_current = gtk_hseparator_new();
			gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_current,
			    			FALSE, FALSE, 0);
		    }
		}
	    }
	    else{ /* if second day and weather data is separated */
		gtk_box_pack_start(GTK_BOX(popup_vbox), create_temperature_range_widget(i),
					FALSE, FALSE, 0);
		separator_after_temperature = gtk_hseparator_new();
		gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_temperature,
					FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(popup_vbox), create_24_hours_widget(i, time(NULL)), /* time(NULL) - current local time */
				    FALSE, FALSE, 0);
		separator_after_24_hours_widget = gtk_hseparator_new();
		gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_24_hours_widget,
				    FALSE, FALSE, 0);
	    }
	}
	else{/* not first or not second day */
	    gtk_box_pack_start(GTK_BOX(popup_vbox), create_temperature_range_widget(i),
				FALSE, FALSE, 0);
	    separator_after_temperature = gtk_hseparator_new();
	    gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_temperature,
				FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(popup_vbox), create_24_hours_widget(i, time(NULL)), /* time(NULL) - current local time */
				FALSE, FALSE, 0);
	    separator_after_24_hours_widget = gtk_hseparator_new();
	    gtk_box_pack_start(GTK_BOX(popup_vbox), separator_after_24_hours_widget,
				FALSE, FALSE, 0);
        }
    }
/* added footer to popup window */
    gtk_box_pack_start(GTK_BOX(popup_vbox), create_footer_widget(),
			    FALSE, FALSE, 0);

    gtk_grab_add(app->popup_window);
    g_signal_connect(G_OBJECT(app->popup_window),
			"button-release-event", 
                        G_CALLBACK(popup_window_event_cb), app->main_window);
    gtk_widget_show_all(app->popup_window);    
}
/*******************************************************************************/
float convert_wind_units(int to, float value){
    float	result = value;
    switch(to){
	    default:
	    case METERS_S: result *= 10.0f / 36.0f; break;
/*	    case KILOMETERS_S: result /= 3600.0f; break;
	    case MILES_S: result /= (1.609344f * 3600.0f); break;
	    case METERS_H: result *= 1000.0f; break;
*/	    case KILOMETERS_H: result *= 1.0f; break;
	    case MILES_H: result /= 1.609344f; break;
	}
    return result;
}
/*******************************************************************************/
GtkWidget* create_header_widget(int i){
    GtkWidget	*main_widget,
		*location_button_hbox,
		*location_label,
		*icon,
		*button,
		*date_hbox,
		*date_label;
    GtkIconInfo *gtkicon_update;
    gchar       buffer[1024];
    struct tm	tmp_time_date_struct;

/* Show full or short name station */ 
    if(i < Max_count_weather_day){
	if(strlen(app->weather_days[i].location) > 2)
	    location_label = gtk_label_new(app->weather_days[i].location);
	else
	    location_label = gtk_label_new(app->config->current_station_name);
    }
    else
	location_label = gtk_label_new(app->config->current_station_name);
/* prepare icon */
    gtkicon_update = gtk_icon_theme_lookup_icon(gtk_icon_theme_get_default(),
        	                        	"qgn_toolb_gene_refresh", 26, 0);
    icon = gtk_image_new_from_file(gtk_icon_info_get_filename(gtkicon_update));
    gtk_icon_info_free(gtkicon_update);
/* prepare button */    
    button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click(GTK_BUTTON(button), FALSE);
    gtk_container_add(GTK_CONTAINER(button), icon);
    gtk_widget_set_events(button, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(button, "clicked",
		    	    G_CALLBACK(pre_update_weather), NULL);      
/* prepare location and button hbox */
    location_button_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(location_button_hbox),
				location_label, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(location_button_hbox),
				button, FALSE, FALSE, 2);
/* prepare date label */
    date_hbox = gtk_hbox_new(FALSE, 0);
    if((i < Max_count_weather_day) && strcmp(app->weather_days[i].date, "") && strcmp(app->weather_days[i].dayfuname, "")){
#ifndef RELEASE
    fprintf(stderr, "\nDate = %s Day name %s\n", app->weather_days[i].date,
		    app->weather_days[i].dayfuname);
#endif
	sprintf(buffer,"%s %s", app->weather_days[i].dayfuname, app->weather_days[i].date);
	strptime(buffer, "%A %b %d", &tmp_time_date_struct);
	memset(buffer, 0, sizeof(buffer));
	strftime(buffer, sizeof(buffer) - 1, "%A %d %B", &tmp_time_date_struct);
        date_label = gtk_label_new(buffer);
	set_font_size(date_label, 16); 
	gtk_box_pack_start(GTK_BOX(date_hbox), date_label, FALSE, FALSE, 5);
    }
/* prepare main vbox */
    main_widget = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_widget),
				location_button_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_widget),
				date_hbox, FALSE, FALSE, 0);
    return main_widget;
}
/*******************************************************************************/
GtkWidget* create_footer_widget(void){
    GtkWidget	*main_widget,
		*label_update,
		*button;
    gchar       buffer[1024],
		full_filename[2048];
    struct stat	statv;
    time_t	tmp_time;

    tmp_time = app->weather_current_day.date_time + 3600;
    
    buffer[0] = 0;    
    snprintf(buffer, sizeof(buffer) - 1, "%s", _("Last update at server: \n"));
    strftime(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer) - 1,
		"%X %x", localtime(&tmp_time));
    strcat(buffer, "\n");
    
    sprintf(full_filename, "%s/%s.xml", app->config->cache_dir_name,
		app->config->current_station_id);
    if(stat(full_filename, &statv)){
    	sprintf(buffer + strlen(buffer), "%s%s",
		_("Last update from server: \n"), _("Unknown"));
	}	
    else{ 
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer) - 1,
		    "%s", _("Last update from server: \n"));
    	strftime(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer) - 1,
		"%X %x", localtime(&statv.st_mtime));
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer) - 1,
		"%s", _(" local time"));

	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer) - 1,
		"Test  %i   %i", app->iap_connected, app->iap_connecting);

    }
    label_update = gtk_label_new(buffer);    
    set_font_size(label_update, 14);
/* prepare Settings button */
    button = gtk_button_new_with_label(_("Settings"));
    set_font_size(button, 14);
    g_signal_connect(button, "clicked",
		    G_CALLBACK(weather_window_settings), NULL);      
/* prepare main widget */
    main_widget = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(main_widget), label_update, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(main_widget), button, FALSE, FALSE, 0);
    return main_widget;
}
/*******************************************************************************/
GtkWidget* create_current_weather_widget(void){
    GtkWidget	*main_widget,
		*temperature_vbox,
		*temperature_label,
		*main_data_vbox,
		*main_data_label,
		*icon_temperature_vbox;
    gchar	buffer[1024],
		*units;
    const gchar	*wind_units_str[] = {	"m/s",
/*					"km/s",
					"mi/s",
					"m/h",
*/					"km/h",
					"mi/h" };
    GdkPixbuf	*icon;
    GtkWidget	*icon_image;
    float	tmp_distance = 0;

    sprintf(buffer,"%s%i.png", path_large_icon, app->weather_current_day.day.icon);
    icon = gdk_pixbuf_new_from_file_at_size(buffer, 64, 64,NULL);
    icon_image = gtk_image_new_from_pixbuf(icon);
    if(icon)
	g_object_unref(icon);

/* prepare current temperature */
    temperature_vbox = gtk_vbox_new(FALSE, 0);
    buffer[0] = 0;
    strcat(buffer, _("Now: "));
    sprintf(buffer + strlen(buffer), "%d\302\260",
		((app->config->temperature_units == CELSIUS) ? ( atoi(app->weather_current_day.day.temp))
							: ( c2f(atoi(app->weather_current_day.day.temp)))));
    (app->config->temperature_units == CELSIUS) ? ( strcat(buffer, _("C")) )
					: ( strcat(buffer, _("F")) );
    temperature_label = gtk_label_new(buffer);
    set_font_size(temperature_label, 14);
    gtk_box_pack_start(GTK_BOX(temperature_vbox), temperature_label, FALSE, FALSE, 0);
    
/* prepare "feels like", "visible", "pressure", "humidity", "wind", "gust" */
/* feels like */
    buffer[0] = 0;
    strcat(buffer, app->weather_current_day.day.title);
    strcat(buffer, _("\nFeels like: "));
    sprintf(buffer + strlen(buffer), "%d\302\260", 
	    (app->config->temperature_units == CELSIUS) ? (atoi(app->weather_current_day.day.temp)) 
						: (c2f(atoi(app->weather_current_day.low_temp))));
    (app->config->temperature_units == CELSIUS) ? ( strcat(buffer, _("C")) )
					: ( strcat(buffer, _("F")) );
/* humidity */
    strcat(buffer, _(" Humidity: "));
    if( strcmp(app->weather_current_day.day.hmid, "N/A") )
	sprintf(buffer + strlen(buffer), "%d%%",
		atoi(app->weather_current_day.day.hmid));
    else
	sprintf(buffer + strlen(buffer), "%s",
		    (char*)hash_table_find((gpointer)"N/A"));
/* visible */
    strcat(buffer, _("\nVisible: "));
    if( !strcmp(app->weather_current_day.day.vis, "Unlimited") )
	sprintf(buffer + strlen(buffer), "%s",
                (char*)hash_table_find((gpointer)"Unlimited"));
    else
	if( strcmp(app->weather_current_day.day.vis, "N/A") ){
	    tmp_distance = atof(app->weather_current_day.day.vis);
	    switch(app->config->distance_units){
		default:
		case METERS: units = _("m"); tmp_distance *= 1000.0f; break;
		case KILOMETERS: units = _("km"); tmp_distance *= 1.0f; break;
		case MILES: units = _("mi"); tmp_distance /= 1.609344f; break;
		case SEA_MILES: units = _("mi"); tmp_distance /= 1.852f; break;
	    }
	    sprintf(buffer + strlen(buffer), "%.2f %s", tmp_distance, units);
	}
	else    
	    sprintf(buffer + strlen(buffer), "%s",
                    (char*)hash_table_find((gpointer)"N/A"));
/* pressure */
    strcat(buffer, _("\nPressure: "));
    sprintf(buffer + strlen(buffer), "%.2f %s, ", app->weather_current_day.day.pressure,
		    _("mb"));
    strcat(buffer, app->weather_current_day.day.pressure_str);

/* wind */
    strcat(buffer, _("\nWind: "));
    if( strcmp(app->weather_current_day.day.wind_speed, "N/A") )	
	    sprintf(buffer + strlen(buffer), "%s %.2f %s", app->weather_current_day.day.wind_title,
		    convert_wind_units(app->config->wind_units, atof(app->weather_current_day.day.wind_speed)),
		    (char*)hash_table_find((gpointer)wind_units_str[app->config->wind_units]));
    else
	sprintf(buffer + strlen(buffer), "%s %s", app->weather_current_day.day.wind_title,
		    (char*)hash_table_find((gpointer)"N/A"));
/* gust */
    strcat(buffer, _(" Gust: "));
    if( strcmp(app->weather_current_day.day.wind_gust, "N/A") )
	    sprintf(buffer + strlen(buffer), "%.2f %s",
		    convert_wind_units(app->config->wind_units, atof(app->weather_current_day.day.wind_gust)),
		    (char*)hash_table_find((gpointer)wind_units_str[app->config->wind_units]));
    else
	strcat(buffer, (char*)hash_table_find((gpointer)"N/A"));

    main_data_vbox = gtk_vbox_new(FALSE, 0);	
    main_data_label = gtk_label_new(buffer);
    set_font_size(main_data_label, 14);
    gtk_box_pack_start(GTK_BOX(main_data_vbox), main_data_label,
			    FALSE, FALSE, 0);
/* prepare icon and temperature vbox */
    icon_temperature_vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(icon_temperature_vbox), temperature_vbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(icon_temperature_vbox), icon_image, FALSE, FALSE, 0);    
/* prepare main widget */
    main_widget = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(main_widget), icon_temperature_vbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_widget), main_data_vbox, FALSE, FALSE, 0);
    return main_widget;
}
/*******************************************************************************/
GtkWidget* create_temperature_range_widget(int i){
    GtkWidget	*main_widget,
		*temperature_title_label,
		*temperature_value_label;
    int		hi_temp,
		low_temp;
    gchar	symbol = 'C',
		buffer[1024];
    
/* prepare temperature */
/* draw temperature, if any of temperature not aviable draw N/A */
    if(!strcmp(app->weather_days[i].hi_temp, "N/A"))
	hi_temp = INT_MAX;
    else
	hi_temp = atoi(app->weather_days[i].hi_temp);
    if(!strcmp(app->weather_days[i].low_temp, "N/A"))
	low_temp = INT_MAX;
    else
	low_temp = atoi(app->weather_days[i].low_temp);
    if(app->config->temperature_units == FAHRENHEIT){
	if(hi_temp != INT_MAX)
	    hi_temp = c2f(hi_temp);
	if(low_temp != INT_MAX)
	    low_temp = c2f(low_temp);
	symbol = 'F';
    }
    if(app->config->swap_hi_low_temperature)
	swap_temperature(&hi_temp, &low_temp);
    buffer[0] = 0;
    /* prepare low temperature */
    if(low_temp == INT_MAX)
	strncat(buffer, (char*)hash_table_find("N/A"),
			( (strlen((char*)hash_table_find("N/A")) > sizeof(buffer)) ?
		        (sizeof(buffer) - 1) :
		        (strlen((char*)hash_table_find("N/A"))) ) );
    else
	snprintf(buffer, sizeof(low_temp) + 
			( (strlen(("\302\260%c")) > sizeof(buffer)) ?
		        (sizeof(buffer) - 1) : (strlen("\302\260%c")) ),
			"%d\302\260%c", low_temp, symbol);
    /* adding separate symbol */
    strncat(buffer, " / ",  
		    ( (strlen((" / ")) > sizeof(buffer)) ?
		      (sizeof(buffer) - 1) : (strlen(" / ")) ) );
    /* prepare hi temperature */
    if(hi_temp == INT_MAX)
	strncat(buffer, (char*)hash_table_find("N/A"),
			( (strlen((char*)hash_table_find("N/A")) > sizeof(buffer)) ?
			(sizeof(buffer) - 1) :
			(strlen((char*)hash_table_find("N/A"))) ) );
    else
	snprintf(buffer + strlen(buffer), sizeof(hi_temp) + 
			( (strlen(("\302\260%c")) > sizeof(buffer) - strlen(buffer)) ?
		        (sizeof(buffer) - strlen(buffer) - 1) : (strlen("\302\260%c")) ),
			"%d\302\260%c", hi_temp, symbol);
    /* prepare temperature hbox */
    temperature_value_label = gtk_label_new(buffer);
    set_font_size(temperature_value_label, 14);
    main_widget = gtk_hbox_new(FALSE, 10);
    temperature_title_label = gtk_label_new(_("Temperature: "));
    set_font_size(temperature_title_label, 14);
    gtk_box_pack_start(GTK_BOX(main_widget), temperature_title_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_widget), temperature_value_label, FALSE, FALSE, 0);
    return main_widget;
}
/*******************************************************************************/
GtkWidget* create_24_hours_widget(int i, time_t current_time){
    GtkWidget	*main_widget,
		*night_hbox,*day_hbox,
		*night_icon_label_vbox, *day_icon_label_vbox,
		*night_icon, *day_icon,
		*night_label, *day_label,
		*night_data_label, *day_data_label,
		*separator_after_night;
    GdkPixbuf	*icon;
    gchar	buffer[1024];
    const gchar	*wind_units_str[] = {	"m/s",
/*					"km/s",
					"mi/s",
					"m/h",
*/					"km/h",
					"mi/h" };
    long int    diff_time;
    time_t 	current_day,utc_time;
    int 	offset = 0 , j = 0;
    struct tm	*tm;

/* prepare additional time values */
    utc_time = mktime(gmtime(&current_time));
    current_time = utc_time + app->weather_days[boxs_offset[i]].zone;
    diff_time = utc_time-current_time + app->weather_days[0].zone;
    current_day = utc_time + app->weather_days[0].zone;
    tm = localtime(&current_day);
    tm->tm_sec = 0; tm->tm_min = 0; tm->tm_hour = 0;
    current_day = mktime(tm);
    offset = (int)( abs( (current_day - app->weather_days[0].date_time) / (24 * 60 * 60) ) );
    (app->config->separate && i == 1) ? (j = -1) : (j = 0); 


/* prepare night data */
    sprintf(buffer, "%s%i.png", path_large_icon, app->weather_days[i].night.icon);
    icon = gdk_pixbuf_new_from_file_at_size(buffer, 64, 64, NULL);
    night_icon = gtk_image_new_from_pixbuf(icon);
    if(icon)
	g_object_unref(icon);
/* prepare night label */
    night_label = gtk_label_new(_("Night:"));
    set_font_size(night_label, 14);
/* preapare night data */
    buffer[0] = 0;
    strcat(buffer, app->weather_days[i].night.title);
    		strcat(buffer, _("\nHumidity: "));
    if( strcmp(app->weather_days[i].night.hmid, "N/A") )
		sprintf(buffer + strlen(buffer), "%d%%\n",
	atoi(app->weather_days[i].night.hmid));
    else
	sprintf(buffer + strlen(buffer), "%s\n",
			(char*)hash_table_find((gpointer)"N/A"));
    strcat(buffer, _("Wind: "));
    sprintf(buffer + strlen(buffer), "%s %.2f %s", app->weather_days[i].night.wind_title,
			convert_wind_units(app->config->wind_units, atof(app->weather_days[i].night.wind_speed)),
			(char*)hash_table_find((gpointer)wind_units_str[app->config->wind_units]));
    night_data_label = gtk_label_new(buffer);    
    set_font_size(night_data_label, 14);
/* prepare icon and label vbox */
    night_icon_label_vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(night_icon_label_vbox), night_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(night_icon_label_vbox), night_icon, FALSE, FALSE, 0);
/* prepare night hbox */
    night_hbox = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(night_hbox), night_icon_label_vbox, FALSE, FALSE, 0);	    
    gtk_box_pack_start(GTK_BOX(night_hbox), night_data_label, FALSE, FALSE, 0);	    

/* prepare day data */
    sprintf(buffer, "%s%i.png", path_large_icon, app->weather_days[i].day.icon);
    icon = gdk_pixbuf_new_from_file_at_size(buffer, 64, 64, NULL);
    day_icon = gtk_image_new_from_pixbuf(icon);
    if(icon)
	g_object_unref(icon);
/* prepare day label */
    day_label = gtk_label_new(_("Day:"));
    set_font_size(day_label, 14);
/* preapare day data */
    buffer[0] = 0;
    strcat(buffer, app->weather_days[i].day.title);
    		strcat(buffer, _("\nHumidity: "));
    if( strcmp(app->weather_days[i].day.hmid, "N/A") )
		sprintf(buffer + strlen(buffer), "%d%%\n",
	atoi(app->weather_days[i].day.hmid));
    else
	sprintf(buffer + strlen(buffer), "%s\n",
			(char*)hash_table_find((gpointer)"N/A"));
    strcat(buffer, _("Wind: "));
    sprintf(buffer + strlen(buffer), "%s %.2f %s", app->weather_days[i].day.wind_title,
			convert_wind_units(app->config->wind_units, atof(app->weather_days[i].day.wind_speed)),
			(char*)hash_table_find((gpointer)wind_units_str[app->config->wind_units]));
    day_data_label = gtk_label_new(buffer);    
    set_font_size(day_data_label, 14);
/* prepare icon and label vbox */
    day_icon_label_vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(day_icon_label_vbox), day_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(day_icon_label_vbox), day_icon, FALSE, FALSE, 0);
/* prepare day hbox */
    day_hbox = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(day_hbox), day_icon_label_vbox, FALSE, FALSE, 0);	    
    gtk_box_pack_start(GTK_BOX(day_hbox), day_data_label, FALSE, FALSE, 0);	    
/* prepare main widget */
    main_widget = gtk_vbox_new(FALSE, 0);
    separator_after_night = gtk_hseparator_new();
/* set the part of firts 24 hours */
/* First icon - morning, day or evening */     
    if((current_time > app->weather_days[i].day.begin_time) &&
    		(current_time < app->weather_days[i].night.begin_time)){
	/* first add day */	
	    gtk_box_pack_start(GTK_BOX(main_widget), day_hbox, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(main_widget), separator_after_night, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(main_widget), night_hbox, FALSE, FALSE, 0);
	} 
	else{
	    if(current_time < app->weather_days[i].night.begin_time){     
	    /* Morning */
		gtk_box_pack_start(GTK_BOX(main_widget), night_hbox, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(main_widget), separator_after_night, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(main_widget), day_hbox, FALSE, FALSE, 0);
	    }
	    else{
		/* Evening */
		    gtk_box_pack_start(GTK_BOX(main_widget), night_hbox, FALSE, FALSE, 0);
		/* Free memory for not used widget */
		    if(day_hbox)
    			gtk_object_destroy(GTK_OBJECT(day_hbox));
		    if(separator_after_night)
			gtk_object_destroy(GTK_OBJECT(separator_after_night));
	    }
	}
    return main_widget;
}
/*******************************************************************************/
GtkWidget* create_sun_time_widget(int i){
    GtkWidget	*main_widget,
		*main_label;
    gchar	buffer[1024],
		time_buffer[1024];
    struct tm	time_show = {0};
    time_t 	current_time,current_day,utc_time;
    int 	offset = 0 , j = 0 , diff_time;
    struct tm	*tm;

/* prepare additional time values */
    current_time = time(NULL);
    utc_time = mktime(gmtime(&current_time));
    current_time = utc_time + app->weather_days[boxs_offset[i]].zone;
    diff_time = utc_time-current_time + app->weather_days[0].zone;
    current_day = utc_time + app->weather_days[0].zone;
    tm = localtime(&current_day);
    tm->tm_sec = 0; tm->tm_min = 0; tm->tm_hour = 0;
    current_day = mktime(tm);
    offset = (int)( abs( (current_day - app->weather_days[0].date_time) / (24 * 60 * 60) ) );
    (app->config->separate && i == 1) ? (j = -1) : (j = 0); 


    memset(buffer, 0, sizeof(buffer));
    memset(time_buffer, 0, sizeof(time_buffer));
    
    snprintf(buffer, sizeof(buffer) - 1, "%s", _("Sunrise: "));
    strptime(app->weather_days[i].sunrise, "%r", &time_show);
    if(strstr(app->weather_days[i].sunrise, "PM"))
	time_show.tm_hour += 12;
    strftime(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer) - 1,
			"%X, ", &time_show);

    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer) - 1,
			"%s", _("Sunset: "));

    memset(time_buffer, 0, sizeof(time_buffer));
    strptime(app->weather_days[i].sunset, "%r", &time_show);
    if(strstr(app->weather_days[i].sunset, "PM"))
	time_show.tm_hour += 12;
    strftime(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer) - 1,
			"%X ", &time_show);

    main_label = gtk_label_new(buffer);
    set_font_size(main_label, 14);
    main_widget = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(main_widget), main_label, FALSE, FALSE, 0);

    return main_widget;
}
/*******************************************************************************/
