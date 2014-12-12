/* Date 8 dec 2014 time 02:35 am 
*
* Copyright 2014 Rachit Goel <rachit2goel@gmail.com>
* http://www.gtk.org/download/linux.php
* gcc cpuusage.c -o cpuusage `pkg-config --libs --cflags gtk+-2.0`
* sudo apt-get install libgtk2.0-dev
* program Version R-1.0
* All right of this code reserved on the name of RTITE <Rachit The IT Engineer>
* This program is to indicate cpuusage as desktop gadget.
*/
#include<gtk/gtk.h>
#include<unistd.h>
#include<math.h>

typedef struct{
  long user;
  long nice;
  long system;
  long idel;
  long iowait;
  long irq;
  long softirq;
}cpu_t;

int get_cpu(cpu_t *cpu){
  char str[BUFSIZ];
  FILE *fp;

  fp = fopen("/proc/stat", "r");
  if(fp == NULL){
    return -1;
  }

  fgets(str, sizeof(str), fp);
  sscanf(str, "cpu %ld %ld %ld %ld %ld %ld %ld",
      &cpu->user, &cpu->nice, &cpu->system, &cpu->idel,
      &cpu->iowait, &cpu->irq, &cpu->softirq);

  fclose(fp);

  return 0;
}

long cpu_all_time(cpu_t *new, cpu_t *old){
  long ret;

  ret = (new->user - old->user) +
    (new->nice - old->nice) +
    (new->system - old->system) +
    (new->idel - old->idel) +
    (new->irq - old->irq) +
    (new->softirq - old->softirq);

  return ret;
}

long cpu_used_time(cpu_t *new, cpu_t *old){
  long ret;

  ret = (new->user - old->user) +
    (new->nice - old->nice) +
    (new->system - old->system) +
    (new->iowait - old->iowait) +
    (new->irq - old->irq) +
    (new->softirq - old->softirq);

  return ret;
}

double cpu_rate(cpu_t *new, cpu_t *old){
  long all, used;
  double rate;

  all = cpu_all_time(new, old);
  used = cpu_used_time(new, old);

  rate = (double)used/all*100;
  if(rate > 100){
    rate = 100;
  }

  return rate;
}

static gint count;
static int ret;
static double rate;
static cpu_t old,new;
static GtkWidget *canvas;

static gint timer_event(gpointer data)
{
	ret = get_cpu(&new);
	if(ret < 0){
	return 1;
	}

	rate = cpu_rate(&new, &old);

	g_printf("cpu rate:%.1f%%\n", rate);

	old = new;

	gtk_widget_queue_draw_area(canvas,0,0,canvas->allocation.width,canvas->allocation.height);
}


gboolean cb_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	GdkWindow *drawable = widget-> window;
	cairo_t *cr;
	double line_width = 30.0;
	double hosuu = 100.0 - rate;
	double hosuu_radian = hosuu/100.0*2.0*M_PI;

	cr = gdk_cairo_create (drawable);
	
	cairo_set_source_rgb(cr,0.0,0.9,1.0);
	cairo_arc(cr,(widget->allocation.width)/2,
			(widget->allocation.height)/2,
			(widget->allocation.width)/2,0.0,2.0*M_PI);
	cairo_fill(cr);

	cairo_set_source_rgb(cr,1.0,1.0,1.0);
	cairo_arc(cr,(widget->allocation.width)/2,
			(widget->allocation.height)/2,
			(widget->allocation.width)/2-60.0,0.0,2.0*M_PI);
	cairo_fill(cr);
	cairo_set_line_width(cr,62.0);
	cairo_arc(cr,(widget->allocation.width)/2,
			(widget->allocation.height)/2,
			(widget->allocation.width)/2-30.0,-M_PI/2.0,-M_PI/2.0+hosuu_radian);
	cairo_stroke(cr);

	char disp[50];
	cairo_set_source_rgb(cr,0.0,0.0,0.0);
	sprintf(disp,"%.1f",rate);
	cairo_move_to(cr,widget->allocation.width/2,widget->allocation.height/2);
	cairo_set_font_size(cr,80.0);
	cairo_show_text(cr,disp);
	cairo_stroke(cr);


	return FALSE;
}

int main(int argc,char** argv)
{

	ret = get_cpu(&old);
	if(ret < 0){
	return 1;
	}

	GtkWidget *window;
	
	
	gtk_init(&argc,&argv);

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_size_request(window,500,500);

	g_signal_connect
		(G_OBJECT(window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
	canvas=gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window),canvas);
	g_signal_connect
		(G_OBJECT(canvas),"expose-event",G_CALLBACK(cb_expose_event),NULL);

	gtk_widget_show_all(window);

	gint my_timer = gtk_timeout_add(2000,(GtkFunction)timer_event,NULL);

	gtk_main();

	return 0;
}
