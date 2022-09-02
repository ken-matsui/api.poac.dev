use crate::{auth, package, user};
use actix_web::web;

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(
        web::scope("/v1")
            .configure(package::init_routes)
            .configure(user::init_routes)
            .configure(auth::init_routes),
    );
}
