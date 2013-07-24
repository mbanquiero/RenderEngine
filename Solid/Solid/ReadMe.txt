================================================================================
    BIBLIOTECA MICROSOFT FOUNDATION CLASS: Información general del proyectoSolid
===============================================================================

El asistente para aplicaciones ha creado esta aplicación Solid. Esta aplicación no solo muestra las bases de la utilización de Microsoft Foundation Classes, también es un punto de partida para escribir la aplicación.

Este archivo contiene un resumen de lo que encontrará en cada uno de los archivos que constituyen la aplicación Solid.

Solid.vcxproj
    Este es el archivo de proyecto principal para los proyectos de VC++ generados mediante un asistente para aplicaciones. Contiene información acerca de la versión de Visual C++ con la que se generó el archivo, así como información acerca de las plataformas, configuraciones y características del proyecto seleccionadas en el asistente para aplicaciones.

Solid.vcxproj.filters
    Éste es el archivo de filtros para los proyectos de VC++ generados mediante un asistente para aplicaciones. Contiene información acerca de la asociación entre los archivos del proyecto y los filtros. Esta asociación se usa en el IDE para mostrar la agrupación de archivos con extensiones similares bajo un nodo específico (por ejemplo, los archivos ".cpp" se asocian con el filtro"Archivos de código fuente").

Solid.h
    Este es el archivo de encabezado principal para la aplicación.
    Incluye otros encabezados específicos del proyecto (incluido Resource.h) y declara la clase de aplicación CSolidApp.

Solid.cpp
    Este es el archivo de código fuente de la aplicación principal que contiene la clase de aplicación CSolidApp.

Solid.rc
    Esta es una lista de todos los recursos de Microsoft Windows que usa el programa. Incluye los iconos, mapas de bits y cursores almacenados en el subdirectorio RES. Este archivo puede editarse directamente en Microsoft Visual C++. Los recursos del proyecto se encuentran en 3082.

res\Solid.ico
    Este es un archivo de icono, que se usa como el icono de la aplicación. Este icono está incluido en el archivo principal de recursos Solid.rc.

res\Solid.rc2
    Este archivo incluye recursos no editados por Microsoft Visual C++. Se deberían colocar en este archivo todos los recursos que no pueden editarse mediante el editor de recursos.

/////////////////////////////////////////////////////////////////////////////

Para la ventana de marco principal:
    El proyecto incluye una interfaz de MFC estándar.

MainFrm.h, MainFrm.cpp
    Estos archivos contienen la clase de marco CMainFrame, que se deriva de
    CFrameWnd y controla todas las características del marco SDI.

res\Toolbar.bmp
    Este archivo de mapa de bits se usa para crear imágenes en mosaico para la barra de herramientas.
    La barra de herramientas inicial y la barra de estado se crean en la clase CMainFrame. Edite este mapa de bits de la barra de herramientas con el editor de recursos y actualice la matriz IDR_MAINFRAME TOOLBAR en Solid.rc para agregar botones de la barra de herramientas.
/////////////////////////////////////////////////////////////////////////////

El asistente para aplicaciones crea un tipo de documento y una vista:

SolidDoc.h, SolidDoc.cpp: el documento
    Estos archivos contienen la clase CSolidDoc. Edite estos archivos para agregar los datos de documentos especiales e implementar el guardado y la carga de archivos (mediante CSolidDoc::Serialize).

SolidView.h, SolidView.cpp: la vista del documento
    Estos archivos contienen la clase CSolidView.
    Los objetos CSolidView se usan para ver objetos CSolidDoc.




/////////////////////////////////////////////////////////////////////////////

Otras características:

Compatibilidad con impresión y vista previa de impresión
    El asistente para aplicaciones ha generado código que controlará los comandos de impresión, configuración de impresión y vista previa de impresión mediante la llamada a funciones miembro de la clase CView de la biblioteca MFC.

/////////////////////////////////////////////////////////////////////////////

Otros archivos estándar:

StdAfx.h, StdAfx.cpp
    Estos archivos se usan para compilar un archivo de encabezado precompilado (PCH) denominado Solid.pch y un archivo de tipos precompilados llamado StdAfx.obj.

Resource.h
    Éste es el archivo de encabezado estándar, que define nuevos identificadores de recurso. Microsoft Visual C++ lee y actualiza este archivo.

Solid.manifest
	Windows XP usa los archivos del manifiesto de aplicación para describir la dependencia de una aplicación de versiones específicas de los ensamblados en paralelo. El cargador usa esta información para cargar el ensamblado adecuado de la memoria caché de ensamblados o de forma privada desde la aplicación. El manifiesto de aplicación puede incluirse para su redistribución como archivo .manifest externo que se instala en la misma carpeta que el ejecutable de la aplicación o puede incluirse en el ejecutable en forma de recurso.
/////////////////////////////////////////////////////////////////////////////

Otras notas:

El asistente para aplicaciones usa "TODO:" para indicar las partes del código fuente que debe agregar o personalizar.

Si la aplicación usa MFC en un archivo DLL compartido, deberá redistribuir los archivos DLL de MFC. Si la aplicación está en un idioma distinto al de la configuración regional del sistema operativo, también deberá redistribuir los recursos mfc110XXX.DLL localizados correspondientes.
Para obtener más información acerca de estos dos temas, vea la sección sobre cómo redistribuir las aplicaciones de Visual C++ en la documentación de MSDN.

/////////////////////////////////////////////////////////////////////////////
