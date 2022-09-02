mod owners;

use actix_web::web::ServiceConfig;

pub(crate) fn init_routes(cfg: &mut ServiceConfig) {
    cfg.configure(owners::init_routes);
}
