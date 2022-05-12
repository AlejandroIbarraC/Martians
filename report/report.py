import random
import matplotlib.pyplot as plt
import matplotlib.ticker as plticker
import pandas as pd
import numpy as np

number_of_grid_lines = 30 

def plot_report(marcianos, tiempos, arribos, nombre_algoritmo):
    ## Ploteo de ejecucion de procesos

    # Random de colores
    colores = {"":(1,1,1)}
    for marciano in marcianos:
        colores[marciano] = (random.random()*0.4+0.5, random.random()*0.4+0.5, random.random()*0.4+0.5)

    # Se agrega el color y id a cada tiempo
    maximo_timeline = 0
    for i, tiempo in enumerate(tiempos):
        maximo_timeline += tiempo['duracion']
        tiempo['color'] = colores[tiempo['marciano']]
        tiempo['id'] = tiempo['marciano']+"_"+str(i)

    # Se crea el diccionario de id con duracion
    data_timeline = {}
    for tiempo in tiempos:
        data_timeline[tiempo['id']] = [tiempo['duracion']]
    
    # Se convierte el diccionario a data_timelineframe
    df = pd.DataFrame(data_timeline)

    # Se prepara el plot
    fig, axes = plt.subplots(nrows=2, ncols=1, constrained_layout=True)
    ax_timeline = df.plot(stacked=True, kind='barh', ax=axes[1], width=0.1)

    # Se agregan los labels

    for i, bar in enumerate(ax_timeline.patches):
        height = bar.get_height()
        width = bar.get_width()
        bar.set_color(tiempos[i]["color"])
        if(tiempos[i]["marciano"] == ""):
            bar.set_hatch(r"//")
            bar.set_color("white")
        bar.set_edgecolor((0.3,0.3,0.3))
        bar.set_linewidth(0.5)
        x = bar.get_x()
        y = bar.get_y()
        label_text = tiempos[i]["marciano"] 
        label_x = x + width / 2
        label_y = y + height / 2
        ax_timeline.text(label_x, label_y, label_text, ha='center',    
                va='center')
        
    # Se agregan los labels de x,y
    Class = [nombre_algoritmo]
    ax_timeline.set_yticklabels(Class,rotation='horizontal')

    ax_timeline.set_title('Tiempos de ejecucion de los procesos')
    ax_timeline.set_xlabel('Tiempo (s)')

    # Se remueve la leyenda

    ax_timeline.get_legend().remove()


    ####################################################################################################################3

    ## Ploteo de procesos listos


    # Se añaden datos de arribo intermedios para los espacios vacios
    new_arribos = []
    last_time_per_marciano = [0 for marciano in marcianos]
    for marciano_id, marciano in enumerate(marcianos):
        for arribo in arribos:
            if marciano == arribo['marciano']:
                new_arribos.append({"marciano":marciano, "duracion":arribo["arribo"]-last_time_per_marciano[marciano_id], 
                "arribo":last_time_per_marciano[marciano_id], "vacio":True})
                new_arribos.append(arribo)
                last_time_per_marciano[marciano_id] = arribo["arribo"]+arribo["duracion"]
    arribos = new_arribos           

    # Se agregan los colores y ids a los arribos
    for i, arribo in enumerate(arribos):
        arribo['color'] = colores[arribo['marciano']]
        arribo['id'] = arribo['marciano']+"_"+str(i)


    # Se crea el diccionario con los datos de arribo 
    data_procesos = {}
    new_arribos = []
    maximos = {}
    for marciano in marcianos:
        maximos[marciano] = 0
    
    for arribo in arribos:
        data_procesos[arribo['id']] = []
        for marciano in marcianos:
            if marciano == arribo['marciano']:
                data_procesos[arribo['id']].append(arribo['duracion'])
                new_arribos.append(arribo)
                maximos[marciano] = maximos[marciano] + arribo['duracion']
            else:
                data_procesos[arribo['id']].append(0)
                new_arribos.append({"vacio":True})

    arribos = new_arribos

    maximo_proceso = 0
    for key in maximos:
        if maximos[key] > maximo_proceso:
            maximo_proceso = maximos[key]

    # Se convierte el diccionario a dataframe
    df = pd.DataFrame(data_procesos)

    # Se prepara el plot
    ax_procesos = df.plot(stacked=True, kind='barh', ax=axes[0])

    # Se agregan los labels
    for i, bar in enumerate(ax_procesos.patches):
        height = bar.get_height()
        width = bar.get_width()
        x = bar.get_x()
        y = bar.get_y()
        if(arribos[i]["vacio"]):
            label_text=""
            bar.set_color("white")
        else:
            label_text = arribos[i]["marciano"]
            bar.set_color(arribos[i]["color"])
            bar.set_edgecolor((0.3,0.3,0.3))
            bar.set_linewidth(0.5)

        label_x = x + width / 2
        label_y = y + height / 2
        ax_procesos.text(label_x, label_y, label_text, ha='center',    
                va='center')
        
    # # Se agregan los labels de x,y
    ax_procesos.set_yticklabels(marcianos,rotation='horizontal')

    ax_procesos.set_title('Tiempos de llegada de los procesos')
    ax_procesos.set_xlabel('Tiempo (s)')
    ax_procesos.set_ylabel('Proceso')

    # Se remueve la leyenda

    ax_procesos.get_legend().remove()

    # Se agregan las lineas de grid
    if maximo_timeline > maximo_proceso:
        maximo = maximo_timeline
    else: 
        maximo = maximo_proceso
    
    ax_timeline.set_xticks(ticks=np.round(np.linspace(0, maximo+1, number_of_grid_lines)))
    ax_procesos.set_xticks(ticks=np.round(np.linspace(0, maximo+1, number_of_grid_lines)))
    ax_procesos.grid(axis = 'x', color = 'gray', linestyle = '--', linewidth = 1)
    ax_timeline.grid(axis = 'x', color = 'gray', linestyle = '--', linewidth = 1)
    plt.xlim((0,maximo+1))

    # Se muestra el grafico
    figManager = plt.get_current_fig_manager()
    figManager.canvas.set_window_title('Reporte')
    plt.show()




