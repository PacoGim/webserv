#ifndef ERROR_HPP
#define ERROR_HPP

namespace Ctx
{
namespace Error
{
	const char content_length_must_be_positive[] = "Content-length must be positive";
    const char content_length_must_be_lower_than[] = "Content-length must be lower than 1Mo";
	const char login_must_have_5_chars[] = "Login pseudo must have at least 5 characters";
    const char internal_error_body_init[] = "Internal Error body init";
	const char internal_error_upload_init[] = "Internal Error upload init";
    const char internal_error_socketpair[] = "Internal Error socket pair init";
    const char internal_error_fork[] = "Internal Error fork init";
    const char internal_error_socketcgi[] = "Internal Error socket cgi init";
    const char id_must_be_a_number[] = "Id must be a positive integer";
    const char id_invalid[] = "Id invalid";
   	const char body_must_begin_by_pseudo[] = "Body must be of format 'pseudo=johndoe'";
    const char pseudo_already_used[] = "Pseudo is already used";
    const char forbidden_cgi[] = "CGI cannot be executed";
    const char forbidden_file[] = "File is protected";
    const char timeout[] = "Request Timeout, please check your connection";
    const char cant_save_uploaded_file[] = "Can't save file";
    const char filename_invalid[] = "Filename invalid";
    const char cgi_failed[] = "CGI failed";
};

} // Ctx

#endif //ERROR_HPP
