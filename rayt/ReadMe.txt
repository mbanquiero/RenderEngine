================================================================================
    BIBLIOTECA MICROSOFT FOUNDATION CLASS: Información general del proyectorayt
===============================================================================

El asistente para aplicaciones ha creado esta aplicación rayt. Esta aplicación no solo muestra las bases de la utilización de Microsoft Foundation Classes, también es un punto de partida para escribir la aplicación.

Este archivo contiene un resumen de lo que encontrará en cada uno de los archivos que constituyen la aplicación rayt.

rayt.vcxproj
    Este es el archivo de proyecto principal para los proyectos de VC++ generados mediante un asistente para aplicaciones. Contiene información acerca de la versión de Visual C++ con la que se generó el archivo, así como información acerca de las plataformas, configuraciones y características del proyecto seleccionadas en el asistente para aplicaciones.

rayt.vcxproj.filters
    Éste es el archivo de filtros para los proyectos de VC++ generados mediante un asistente para aplicaciones. Contiene información acerca de la asociación entre los archivos del proyecto y los filtros. Esta asociación se usa en el IDE para mostrar la agrupación de archivos con extensiones similares bajo un nodo específico (por ejemplo, los archivos ".cpp" se asocian con el filtro"Archivos de código fuente").

rayt.h
    Este es el archivo de encabezado principal para la aplicación.
    Incluye otros encabezados específicos del proyecto (incluido Resource.h) y declara la clase de aplicación CraytApp.

rayt.cpp
    Este es el archivo de código fuente de la aplicación principal que contiene la clase de aplicación CraytApp.

rayt.rc
    Esta es una lista de todos los recursos de Microsoft Windows que usa el programa. Incluye los iconos, mapas de bits y cursores almacenados en el subdirectorio RES. Este archivo puede editarse directamente en Microsoft Visual C++. Los recursos del proyecto se encuentran en 3082.

res\rayt.ico
    Este es un archivo de icono, que se usa como el icono de la aplicación. Este icono está incluido en el archivo principal de recursos rayt.rc.

res\rayt.rc2
    Este archivo incluye recursos no editados por Microsoft Visual C++. Se deberían colocar en este archivo todos los recursos que no pueden editarse mediante el editor de recursos.


/////////////////////////////////////////////////////////////////////////////

El asistente para aplicaciones crea una clase de cuadro de diálogo:

raytDlg.h, raytDlg.cpp: el cuadro de diálogo
    Estos archivos contienen la clase CraytDlg. Esta clase define el comportamiento del cuadro de diálogo principal de la aplicación. La plantilla del cuadro de diálogo se encuentra en rayt.rc, que puede editarse en Microsoft Visual C++.

/////////////////////////////////////////////////////////////////////////////

Otros archivos estándar:

StdAfx.h, StdAfx.cpp
    Estos archivos se usan para compilar un archivo de encabezado precompilado (PCH) denominado rayt.pch y un archivo de tipos precompilados llamado StdAfx.obj.

Resource.h
    Éste es el archivo de encabezado estándar, que define nuevos identificadores de recurso. Microsoft Visual C++ lee y actualiza este archivo.

/////////////////////////////////////////////////////////////////////////////

Otras notas:

El asistente para aplicaciones usa "TODO:" para indicar las partes del código fuente que debe agregar o personalizar.

Si la aplicación usa MFC en un archivo DLL compartido, deberá redistribuir los archivos DLL de MFC. Si la aplicación está en un idioma distinto al de la configuración regional del sistema operativo, también deberá redistribuir los recursos mfc110XXX.DLL localizados correspondientes.
Para obtener más información acerca de estos dos temas, vea la sección sobre cómo redistribuir las aplicaciones de Visual C++ en la documentación de MSDN.

/////////////////////////////////////////////////////////////////////////////
