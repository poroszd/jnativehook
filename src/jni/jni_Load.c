/* JNativeHook: Global keyboard and mouse hooking for Java.
 * Copyright (C) 2006-2013 Alexander Barker.  All Rights Received.
 * http://code.google.com/p/jnativehook/
 *
 * JNativeHook is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JNativeHook is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <jni.h>
#include <uiohook.h>

#include "jni_Errors.h"
#include "jni_EventDispathcer.h"
#include "jni_Globals.h"
#include "jni_Logger.h"
#include "jni_Properties.h"

#ifdef USE_QUIET
#define COPYRIGHT() (void) 0;
#else
#include <stdio.h>
#define COPYRIGHT()	fprintf(stdout, \
		"JNativeHook: Global keyboard and mouse hooking for Java.\n" \
		"Copyright (C) 2006-2014 Alexander Barker.  All Rights Received.\n" \
		"https://github.com/kwhat/libuiohook/\n" \
		"\n" \
		"JNativeHook is free software: you can redistribute it and/or modify\n" \
		"it under the terms of the GNU Lesser General Public License as published\n" \
		"by the Free Software Foundation, either version 3 of the License, or\n" \
		"(at your option) any later version.\n" \
		"\n" \
		"JNativeHook is distributed in the hope that it will be useful,\n" \
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
		"GNU General Public License for more details.\n" \
		"\n" \
		"You should have received a copy of the GNU Lesser General Public License\n" \
		"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n");
#endif

// JNI Related global references.
JavaVM *jvm;
jint jni_version = JNI_VERSION_1_4;

// JNI entry point, This is executed when the Java virtual machine attaches to the native library.
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	// Display the copyright on library load.
	COPYRIGHT();

	/* Grab the currently running virtual machine so we can attach to it in
	 * functions that are not called from java.
	 */
	jvm = vm;
	JNIEnv *env = NULL;
	if ((*jvm)->GetEnv(jvm, (void **)(&env), jni_version) == JNI_OK) {
		#ifdef DEBUG
		fprintf(stdout, "%s [%u]: GetEnv() successful.\n", 
				__FUNCTION__, __LINE__);
		#endif

		// Create all the global class references onload to prevent class loader
		// issues with JNLP and some IDE's.
		// FIXME Change to take jvm, not env!
		if (jni_CreateGlobals(env) != JNI_OK) {
			#ifdef DEBUG
			fprintf(stderr, "%s [%u]: CreateJNIGlobals() failed!\n", 
					__FUNCTION__, __LINE__);
			#endif

			ThrowFatalError("Failed to locate one or more required classes.");
		}

		// Set java properties from native sources.
		jni_SetProperties(env);

		// Set Java logger for native code messages.
		hook_set_logger_proc(&jni_Logger);

		// Set the hook callback function to dispatch events.
		hook_set_dispatch_proc(&jni_EventDispatcher);
	}
	else {
		#ifdef DEBUG
		fprintf(stderr, "%s [%u]: GetEnv() failed!\n", 
				__FUNCTION__, __LINE__);
		#endif

		ThrowFatalError("Failed to aquire JNI interface pointer");
	}

	#ifdef DEBUG
	fprintf(stdout, "%s [%u]: JNI Loaded.\n", 
			__FUNCTION__, __LINE__);
	#endif

    return jni_version;
}

// JNI exit point, This is executed when the Java virtual machine detaches from the native library.
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
	// Grab the currently JNI interface pointer so we can cleanup the
	// system properties set on load.
	JNIEnv *env = NULL;
	if ((*jvm)->GetEnv(jvm, (void **)(&env), jni_version) == JNI_OK) {
		// Clear java properties from native sources.
		// FIXME Change to take jvm, not env!
		jni_ClearProperties(env);
	}
	#ifdef DEBUG
	else {
		// It is not critical that these values are cleared so no exception
		// will be thrown.
		fprintf(stdout, "%s [%u]: GetEnv() failed!\n", 
				__FUNCTION__, __LINE__);
	}
	#endif

	jni_DestroyGlobals(env);

	#ifdef DEBUG
	fprintf(stdout, "JNI_OnUnload(): JNI Unloaded.\n");
	#endif
}
