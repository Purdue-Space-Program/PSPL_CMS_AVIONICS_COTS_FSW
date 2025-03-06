use std::sync::{Arc, RwLock};
use std::thread;

use tokio::runtime;
use tonic::{Request, Response, Status, transport::Server};

use command_service::command_service_server::{CommandService, CommandServiceServer};
use command_service::{Empty, SetpointRequest, State, StateRequest};

use crate::bang_bang::{ControlState, SystemConfig};

pub mod command_service {
    tonic::include_proto!("command");
}

impl Into<ControlState> for State {
    fn into(self) -> ControlState {
        match self {
            State::Isolate => ControlState::Isolate,
            State::Regulate => ControlState::Regulate,
            State::Open => ControlState::Open,
        }
    }
}

#[derive(Debug)]
pub struct MyCommandService {
    fu_config: Arc<RwLock<SystemConfig>>,
    ox_config: Arc<RwLock<SystemConfig>>,
}

impl MyCommandService {
    pub fn new(fu_config: Arc<RwLock<SystemConfig>>, ox_config: Arc<RwLock<SystemConfig>>) -> Self {
        MyCommandService {
            fu_config,
            ox_config,
        }
    }
}

#[tonic::async_trait]
impl CommandService for MyCommandService {
    async fn set_fu_upper_setpoint(
        &self,
        req: Request<SetpointRequest>,
    ) -> Result<Response<Empty>, Status> {
        let usp = req.into_inner().value;
        let fu_config = self.fu_config.clone();
        tokio::task::spawn_blocking(move || {
            let mut config = fu_config.write().unwrap();
            config.upper_setpoint = usp;
        })
        .await
        .unwrap();
        Ok(Response::new(Empty {}))
    }
    async fn set_fu_lower_setpoint(
        &self,
        req: Request<SetpointRequest>,
    ) -> Result<Response<Empty>, Status> {
        let lsp = req.into_inner().value;
        let fu_config = self.fu_config.clone();
        tokio::task::spawn_blocking(move || {
            let mut config = fu_config.write().unwrap();
            config.lower_setpoint = lsp;
        })
        .await
        .unwrap();
        Ok(Response::new(Empty {}))
    }
    async fn set_ox_upper_setpoint(
        &self,
        req: Request<SetpointRequest>,
    ) -> Result<Response<Empty>, Status> {
        let usp = req.into_inner().value;
        let ox_config = self.ox_config.clone();
        tokio::task::spawn_blocking(move || {
            let mut config = ox_config.write().unwrap();
            config.upper_setpoint = usp;
        })
        .await
        .unwrap();
        Ok(Response::new(Empty {}))
    }
    async fn set_ox_lower_setpoint(
        &self,
        req: Request<SetpointRequest>,
    ) -> Result<Response<Empty>, Status> {
        let lsp = req.into_inner().value;
        let ox_config = self.ox_config.clone();
        tokio::task::spawn_blocking(move || {
            let mut config = ox_config.write().unwrap();
            config.lower_setpoint = lsp;
        })
        .await
        .unwrap();
        Ok(Response::new(Empty {}))
    }

    async fn set_fu_state(&self, req: Request<StateRequest>) -> Result<Response<Empty>, Status> {
        let state = req.into_inner().state();
        let fu_config = self.fu_config.clone();
        tokio::task::spawn_blocking(move || {
            let mut config = fu_config.write().unwrap();
            config.control_state = state.into();
        })
        .await
        .unwrap();
        Ok(Response::new(Empty {}))
    }
    async fn set_ox_state(&self, req: Request<StateRequest>) -> Result<Response<Empty>, Status> {
        let state = req.into_inner().state();
        let ox_config = self.ox_config.clone();
        tokio::task::spawn_blocking(move || {
            let mut config = ox_config.write().unwrap();
            config.control_state = state.into();
        })
        .await
        .unwrap();
        Ok(Response::new(Empty {}))
    }
    async fn set_bb_state(&self, req: Request<StateRequest>) -> Result<Response<Empty>, Status> {
        let state = req.into_inner().state();
        let fu_config = self.fu_config.clone();
        let ox_config = self.ox_config.clone();
        tokio::task::spawn_blocking(move || {
            let mut fu_config = fu_config.write().unwrap();
            fu_config.control_state = state.into();
            let mut ox_config = ox_config.write().unwrap();
            ox_config.control_state = state.into();
        })
        .await
        .unwrap();
        Ok(Response::new(Empty {}))
    }

    async fn noop(&self, _: Request<Empty>) -> Result<Response<Empty>, Status> {
        println!("command server: got noop");
        Ok(Response::new(Empty {}))
    }
    async fn start(&self, _: Request<Empty>) -> Result<Response<Empty>, Status> {
        println!("command server: got start");
        Ok(Response::new(Empty {}))
    }
    async fn abort(&self, _: Request<Empty>) -> Result<Response<Empty>, Status> {
        println!("command server: got abort");
        Ok(Response::new(Empty {}))
    }
}

pub fn start_command_server(
    fu_config: Arc<RwLock<SystemConfig>>,
    ox_config: Arc<RwLock<SystemConfig>>,
) {
    thread::spawn(move || {
        let rt = runtime::Builder::new_current_thread().build().unwrap();
        rt.block_on(command_server(fu_config, ox_config));
    });
}

async fn command_server(
    fu_config: Arc<RwLock<SystemConfig>>,
    ox_config: Arc<RwLock<SystemConfig>>,
) {
    let addr = "[::1]:25565".parse().unwrap();
    let svc = MyCommandService::new(fu_config, ox_config);

    Server::builder()
        .add_service(CommandServiceServer::new(svc))
        .serve(addr)
        .await
        .unwrap();
}
