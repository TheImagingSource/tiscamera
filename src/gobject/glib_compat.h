#ifndef __GLIB_COMPAT_H__
#define __GLIB_COMPAT_H__

#ifndef G_DECLARE_INTERFACE
/**
 * G_DECLARE_INTERFACE:
 * @ModuleObjName: The name of the new type, in camel case (like GtkWidget)
 * @module_obj_name: The name of the new type in lowercase, with words
 *  separated by '_' (like 'gtk_widget')
 * @MODULE: The name of the module, in all caps (like 'GTK')
 * @OBJ_NAME: The bare name of the type, in all caps (like 'WIDGET')
 * @PrerequisiteName: the name of the prerequisite type, in camel case (like GtkWidget)
 *
 * A convenience macro for emitting the usual declarations in the header file for a GInterface type.
 *
 * You might use it in a header as follows:
 *
 * |[
 * #ifndef _my_model_h_
 * #define _my_model_h_
 *
 * #define MY_TYPE_MODEL my_model_get_type ()
 * GDK_AVAILABLE_IN_3_12
 * G_DECLARE_INTERFACE (MyModel, my_model, MY, MODEL, GObject)
 *
 * struct _MyModelInterface
 * {
 *   GTypeInterface g_iface;
 *
 *   gpointer (* get_item)  (MyModel *model);
 * };
 *
 * gpointer my_model_get_item (MyModel *model);
 *
 * ...
 *
 * #endif
 * ]|
 *
 * This results in the following things happening:
 *
 * - the usual my_model_get_type() function is declared with a return type of #GType
 *
 * - the MyModelInterface type is defined as a typedef to struct _MyModelInterface,
 *   which is left undefined. You should do this from the header file directly after
 *   you use the macro.
 *
 * - the MY_MODEL() cast is emitted as static inline functions along with
 *   the MY_IS_MODEL() type checking function and MY_MODEL_GET_IFACE() function.
 *
 * - g_autoptr() support being added for your type, based on your prerequisite type.
 *
 * You can only use this function if your prerequisite type also supports g_autoptr().
 *
 * Because the type macro (MY_TYPE_MODEL in the above example) is not a callable, you must continue to
 * manually define this as a macro for yourself.
 *
 * The declaration of the _get_type() function is the first thing emitted by the macro.  This allows this macro
 * to be used in the usual way with export control and API versioning macros.
 *
 * Since: 2.44
 **/
#define G_DECLARE_INTERFACE(ModuleObjName, module_obj_name, MODULE, OBJ_NAME, PrerequisiteName) \
  GType module_obj_name##_get_type (void);                                                                 \
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS                                                                         \
  typedef struct _##ModuleObjName ModuleObjName;                                                           \
  typedef struct _##ModuleObjName##Interface ModuleObjName##Interface;                                     \
                                                                                                           \
  _GLIB_DEFINE_AUTOPTR_CHAINUP (ModuleObjName, PrerequisiteName)                                           \
                                                                                                           \
  static inline ModuleObjName * MODULE##_##OBJ_NAME (gconstpointer ptr) {                                       \
    return G_TYPE_CHECK_INSTANCE_CAST (ptr, module_obj_name##_get_type (), ModuleObjName); }               \
  static inline gboolean MODULE##_IS_##OBJ_NAME (gconstpointer ptr) {                                           \
    return G_TYPE_CHECK_INSTANCE_TYPE (ptr, module_obj_name##_get_type ()); }                              \
  static inline ModuleObjName##Interface * MODULE##_##OBJ_NAME##_GET_IFACE (gconstpointer ptr) {                \
    return G_TYPE_INSTANCE_GET_INTERFACE (ptr, module_obj_name##_get_type (), ModuleObjName##Interface); } \
  G_GNUC_END_IGNORE_DEPRECATIONS

#endif
#endif// __GLIB_COMPAT_H__
