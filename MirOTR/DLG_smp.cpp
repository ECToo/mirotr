#include "stdafx.h"
#include <map>
//TODO: Social Millionaire Protocol
typedef std::map<HANDLE, HWND> SmpForContactMap;
SmpForContactMap smp_for_contact;

HWND smp_find_for_contact(HANDLE hContact) {
	SmpForContactMap::iterator iter = smp_for_contact.find(hContact);
	if (iter == smp_for_contact.end()) return null;
	return iter->second;
}

/* Create the SMP dialog.  responder is true if this is called in
 * response to someone else's run of SMP. */
static void dialog_socialist_millionaires(ConnContext *context,
	TCHAR *question, bool responder)
{
    if (context == NULL) return;
	TCHAR primary[1024];

    if (responder && question) {
		(HANDLE)context->app_data
        mir_sntprintf(primary, 1024, TranslateT(LANG_SMP_AUTH_FROM),
            contact_get_nameT((HANDLE)context->app_data));
    } else {
        mir_sntprintf(primary, 1024, TranslateT(LANG_SMP_AUTH),
            contact_get_nameT((HANDLE)context->app_data));
    }
    
    /* fprintf(stderr, "Question = ``%s''\n", question); */
	//TCHAR* proto_name = mir_a2t(context->protocol);
	//if (!proto_name) proto_name = mir_tstrdup(TranslateT(LANG_UNKNOWN));
    

    dialog = create_smp_dialog(_("Authenticate Buddy"),
	    primary, context, responder, question);

    //mir_free(proto_name);
}

/* Call this to update the status of an ongoing socialist millionaires
 * protocol.  Progress_level is a percentage, from 0.0 (aborted) to
 * 1.0 (complete).  Any other value represents an intermediate state. */
static void otrg_gtk_dialog_update_smp(ConnContext *context,
	double progress_level)
{
    PurpleConversation *conv = otrg_plugin_context_to_conv(context, 0);
    GtkProgressBar *bar;
    SMPData *smp_data = purple_conversation_get_data(conv, "otr-smpdata");

    if (!smp_data) return;

    bar = GTK_PROGRESS_BAR(smp_data->smp_progress_bar);
    gtk_progress_bar_set_fraction(bar, progress_level);

    /* If the counter is reset to absolute zero, the protocol has aborted */
    if (progress_level == 0.0) {
        GtkDialog *dialog = GTK_DIALOG(smp_data->smp_progress_dialog);

	gtk_dialog_set_response_sensitive(dialog, GTK_RESPONSE_ACCEPT, 1);
	gtk_dialog_set_response_sensitive(dialog, GTK_RESPONSE_REJECT, 0);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
		GTK_RESPONSE_ACCEPT);

	gtk_label_set_text(GTK_LABEL(smp_data->smp_progress_label),
		_("An error occurred during authentication."));
	return;
    } else if (progress_level == 1.0) {
	/* If the counter reaches 1.0, the protocol is complete */
        GtkDialog *dialog = GTK_DIALOG(smp_data->smp_progress_dialog);

	gtk_dialog_set_response_sensitive(dialog, GTK_RESPONSE_ACCEPT, 1);
	gtk_dialog_set_response_sensitive(dialog, GTK_RESPONSE_REJECT, 0);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
		GTK_RESPONSE_ACCEPT);

        if (context->smstate->sm_prog_state == OTRL_SMP_PROG_SUCCEEDED) {
	    if (context->active_fingerprint->trust &&
		    context->active_fingerprint->trust[0]) {
		gtk_label_set_text(GTK_LABEL(smp_data->smp_progress_label),
			_("Authentication successful."));
	    } else {
		gtk_label_set_text(GTK_LABEL(smp_data->smp_progress_label),
			_("Your buddy has successfully authenticated you.  "
			    "You may want to authenticate your buddy as "
			    "well by asking your own question."));
	    }
        } else {
	    gtk_label_set_text(GTK_LABEL(smp_data->smp_progress_label),
		    _("Authentication failed."));
	}
    } else {
	/* Clear the progress label */
	gtk_label_set_text(GTK_LABEL(smp_data->smp_progress_label), "");
    }
}