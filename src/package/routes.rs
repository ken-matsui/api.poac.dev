mod dependents;
mod deps;
mod get_all;
mod owned_packages;
mod repo_info;
pub(crate) mod search;
mod versions;

use actix_web::web::ServiceConfig;

pub(crate) fn init_routes(cfg: &mut ServiceConfig) {
    cfg.configure(get_all::init_routes);
    cfg.configure(search::init_routes);
    cfg.configure(repo_info::init_routes);
    cfg.configure(versions::init_routes);
    cfg.configure(deps::init_routes);
    cfg.configure(dependents::init_routes);
    cfg.configure(owned_packages::init_routes);
}
