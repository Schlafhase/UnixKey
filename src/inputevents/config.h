#include <fcitx-config/configuration.h>
#include <fcitx-config/option.h>
#include <fcitx-utils/i18n.h>
FCITX_CONFIGURATION(UnixKeyConfig,
                    fcitx::Option<bool> usePreedit{this, "UsePreedit",
                                                   _("Use preedit"), false};);
