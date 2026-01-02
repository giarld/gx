//
// Created by Gxin on 2022/10/31.
//

#include <gx/gany.h>

#include <gx/ghashsum.h>


using namespace gany;

void refGHashSum()
{
    Class<GHashJob>("Gx", "GHashJob", "Gx HashSum job.")
            .func("update", [](GHashJob &self, const GByteArray &data) {
                self.update(data);
            })
            .func("update", [](GHashJob &self, GAnyConstBytePtr *data, uint32_t size) {
                self.update(reinterpret_cast<const uint8_t *>(data), size);
            })
            .func("final", &GHashJob::final);

    Class<GHashSum::HashType>("Gx", "HashType", "")
            .defEnum({
                         {"Md5", GHashSum::Md5},
                         {"Sha1", GHashSum::Sha1},
                         {"Sha256", GHashSum::Sha256},
                     })
            .func(MetaFunction::ToString, [](GHashSum::HashType &self) {
                switch (self) {
                    case GHashSum::Md5:
                        return "Md5";
                    case GHashSum::Sha1:
                        return "Sha1";
                    case GHashSum::Sha256:
                        return "Sha256";
                }
                return "";
            })
            REF_ENUM_OPERATORS(GHashSum::HashType);

    Class<GHashSum>("Gx", "GHashSum", "Gx HashSum.")
            .staticFunc("hashSum", &GHashSum::hashSum)
            .defEnum({
                         {"Md5", GHashSum::Md5},
                         {"Sha1", GHashSum::Sha1},
                         {"Sha256", GHashSum::Sha256},
                     });
}
