class ApplicationController < ActionController::Base
  before_action :verify_login

  def login_user(user_id)
    reset_session
    session[:user_id] = user_id
  end

  def verify_login
    begin
      user = User.find(session[:user_id])
    rescue ActiveRecord::RecordNotFound
      redirect_to login_path
    end
  end

end
